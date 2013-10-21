#include <stdio.h>      /* printf, scanf, NULL */
#include <string.h>
#include <ev.h>
#include <glog/logging.h>
#include <Server.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char * argv[])
{
	FLAGS_logtostderr = true;
	FLAGS_colorlogtostderr = true;
	google::InitGoogleLogging(argv[0]);

	int c_port_no, w_port_no;

	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "produce help message.")
		("clients-port,cp", po::value<int>(&c_port_no)->default_value(1025), "The port to listen clients on. Must be greater than 1024. Default is 1025.")
		("worlds-port,wp", po::value<int>(&w_port_no)->default_value(1026), "The port to listen worlds on. Must be greater than 1024. Default is 1026.")
	;

	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, desc), vm);
	} catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::unknown_option> > e) {
		std::cout << "Unknown parameters" << std::endl;
		std::cout << desc << std::endl;
		return 1;
	} catch (...) {
		std::cout << desc << std::endl;
		return 1;
	}
	po::notify(vm);

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 1;
	}

	if (c_port_no <= 1024 || w_port_no <= 1024) {
		std::stringstream str;
		str << desc;
		fprintf(stderr, " bad port number: %i\n usage: %s [options]\n%s\n", c_port_no <= 1024 ? c_port_no : w_port_no, argv[0], str.str().c_str());
		return 1;
	}

	ev::default_loop loop;
	Server::Instance().start(c_port_no, w_port_no);
	loop.run(0);
	return 0;
}
