#include <iostream>

#include <array>
#include <cmath>
#include <fstream>
#include <random>

#include <aux/json_params.hpp>

struct nest_params {
    nest_params() = default;

    unsigned num_cells = 10;
    double min_delay = 7;
};

nest_params read_options_nest(int argc, char** argv) {
    using aux::param_from_json;

    nest_params params;
    if (argc<2) {
        return params;
    }
    if (argc>2) {
        throw std::runtime_error("More than command line one option not permitted.");
    }

    std::string fname = argv[1];
    std::cout << "Loading parameters from file: " << fname << "\n";
    std::ifstream f(fname);

    if (!f.good()) {
        throw std::runtime_error("Unable to open input parameter file: "+fname);
    }

    nlohmann::json json;
    json << f;

    param_from_json(params.num_cells, "num-cells", json);
    param_from_json(params.min_delay, "min-delay", json);

    if (!json.empty()) {
        for (auto it=json.begin(); it!=json.end(); ++it) {
            std::cout << "  Warning: unused input parameter: \"" << it.key() << "\"\n";
        }
        std::cout << "\n";
    }

    return params;
}
