#include "math_core.hpp"
#include <iostream>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>

#if defined(__linux__)
#include <sys/resource.h>
#endif

namespace {
long get_peak_memory_bytes() {
#if defined(__linux__)
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        // ru_maxrss est en kilo-octets sous Linux
        return usage.ru_maxrss * 1024L;
    }
#endif
    return 0L;
}
}

int main(int argc, char* argv[]) {
    std::string job_id = "00000000-0000-0000-0000-000000000000";
    std::string algorithm = "monte_carlo_pi";
    std::size_t dimensions = 1000;
    std::size_t iterations = 100000;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--job-id" && i + 1 < argc) {
            job_id = argv[++i];
        } else if (arg == "--algorithm" && i + 1 < argc) {
            algorithm = argv[++i];
        } else if (arg == "--dimensions" && i + 1 < argc) {
            dimensions = std::stoull(argv[++i]);
        } else if (arg == "--iterations" && i + 1 < argc) {
            iterations = std::stoull(argv[++i]);
        }
    }

    auto start = std::chrono::high_resolution_clock::now();
    double summary_value = 0.0;
    std::string status = "completed";
    std::string error_msg = "";

    try {
        if (algorithm == "monte_carlo_pi") {
            summary_value = forge::native::monte_carlo_pi(iterations, 42);
        } else if (algorithm == "vector_dot_product") {
            std::vector<double> v1(dimensions, 1.001);
            std::vector<double> v2(dimensions, 0.999);
            for (std::size_t iter = 0; iter < iterations; ++iter) {
                summary_value = forge::native::dot_product(v1, v2);
            }
        } else if (algorithm == "matrix_multiply") {
            std::size_t n = dimensions;
            if (n > 1000) n = 1000; // Sécurité de dimension
            std::vector<double> a(n * n, 1.0);
            std::vector<double> b(n * n, 2.0);
            auto c = forge::native::square_matrix_multiply(a, b, n);
            summary_value = c.empty() ? 0.0 : c[0];
        } else {
            status = "failed";
            error_msg = "Algorithme non supporté : " + algorithm;
        }
    } catch (const std::exception& e) {
        status = "failed";
        error_msg = e.what();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;

    std::ostringstream checksum;
    checksum << "chk_" << std::hex << static_cast<long long>(summary_value * 100000.0);

    // Sortie au format JSON strict conforme à ComputeJobResponseSchema dans contracts/schemas.ts
    std::cout << "{\n"
              << "  \"jobId\": \"" << job_id << "\",\n"
              << "  \"status\": \"" << status << "\",\n"
              << "  \"executionTimeMs\": " << std::fixed << std::setprecision(3) << duration.count() << ",\n"
              << "  \"peakMemoryBytes\": " << get_peak_memory_bytes() << ",\n"
              << "  \"resultChecksum\": \"" << checksum.str() << "\",\n"
              << "  \"summaryValue\": " << std::setprecision(8) << summary_value;
    if (!error_msg.empty()) {
        std::cout << ",\n  \"error\": \"" << error_msg << "\"";
    }
    std::cout << "\n}\n";

    return (status == "completed") ? 0 : 1;
}
