#include <cstdio>
#include <chrono>
#include <random>
#include <set>
#include <vector>
#include <algorithm>
#include "rb_set.h"

using namespace std::chrono;
double to_us(auto d) { return duration<double, std::micro>(d).count(); }

struct Stats { double avg, min, max; };
Stats compute(std::vector<double>& v)
{
    std::sort(v.begin(), v.end());
    double sum = 0; for (auto x : v) sum += x;
    return { sum / v.size(), v.front(), v.back() };
}

int main()
{
    const int ROUNDS = 50;
    const int N = 500000;
    std::mt19937 base_rng{ 42 };
    std::vector<double> rb_times, std_times;
    
#ifdef __GNUC__
    printf("=== Yc::rb_set vs std::set (MinGW g++ %d.%d -O2) ===\n", __GNUC__, __GNUC_MINOR__);
#else
    printf("=== Yc::rb_set vs std::set (MSVC -O2) ===\n");
#endif
    printf("  N=%d  rounds=%d\n", N, ROUNDS);
    printf("  %-18s | %10s | %10s | %8s | %8s\n", "Test", "rb(ms)", "std(ms)", "Ratio", "rb range");
    printf("  %s-+-%s-+-%s-+-%s-+-%s\n", "------------------", "----------", "----------", "--------", "--------");
    
    // === Insert random ===
    {
        std::vector<int> data(N);
        for (int r = 0; r < ROUNDS; ++r)
        {
            base_rng.seed(42 + r * 1000);
            for (auto& v : data) v = base_rng();
            
            auto t0 = high_resolution_clock::now();
            { std::set<int> s; for (int v : data) s.insert(v); }
            auto t1 = high_resolution_clock::now();
            
            base_rng.seed(42 + r * 1000);
            for (auto& v : data) v = base_rng();
            
            auto t2 = high_resolution_clock::now();
            { Yc::rb_set<int> rb; for (int v : data) rb.insert(v); }
            auto t3 = high_resolution_clock::now();
            
            std_times.push_back(to_us(t1 - t0) / 1000.0);
            rb_times.push_back(to_us(t3 - t2) / 1000.0);
        }
        auto rb_s = compute(rb_times), std_s = compute(std_times);
        printf("  %-18s | %10.1f | %10.1f | %7.2fx | %.0f-%.0f\n",
            "insert_random", rb_s.avg, std_s.avg, std_s.avg/rb_s.avg, rb_s.min, rb_s.max);
        fflush(stdout);
    }
    
    // === Insert sorted ===
    {
        rb_times.clear(); std_times.clear();
        for (int r = 0; r < ROUNDS; ++r)
        {
            auto t0 = high_resolution_clock::now();
            { std::set<int> s; for (int i = 0; i < N; ++i) s.insert(i); }
            auto t1 = high_resolution_clock::now();
            auto t2 = high_resolution_clock::now();
            { Yc::rb_set<int> rb; for (int i = 0; i < N; ++i) rb.insert(i); }
            auto t3 = high_resolution_clock::now();
            std_times.push_back(to_us(t1 - t0) / 1000.0);
            rb_times.push_back(to_us(t3 - t2) / 1000.0);
        }
        auto rb_s = compute(rb_times), std_s = compute(std_times);
        printf("  %-18s | %10.1f | %10.1f | %7.2fx | %.0f-%.0f\n",
            "insert_sorted", rb_s.avg, std_s.avg, std_s.avg/rb_s.avg, rb_s.min, rb_s.max);
        fflush(stdout);
    }
    
    // === Find ===
    {
        rb_times.clear(); std_times.clear();
        std::vector<int> data(N), queries(N);
        for (int r = 0; r < ROUNDS; ++r)
        {
            base_rng.seed(42 + r * 1000);
            for (auto& v : data) v = base_rng();
            Yc::rb_set<int> rb; std::set<int> s;
            for (int v : data) { rb.insert(v); s.insert(v); }
            
            base_rng.seed(99 + r * 1000);
            for (auto& v : queries) v = base_rng();
            volatile long long found = 0;
            
            auto t0 = high_resolution_clock::now();
            for (int q : queries) if (s.find(q) != s.end()) found += 1;
            auto t1 = high_resolution_clock::now();
            for (int q : queries) if (rb.find(q) != rb.end()) found += 1;
            auto t2 = high_resolution_clock::now();
            (void)found;
            std_times.push_back(to_us(t1 - t0) / 1000.0);
            rb_times.push_back(to_us(t2 - t1) / 1000.0);
        }
        auto rb_s = compute(rb_times), std_s = compute(std_times);
        printf("  %-18s | %10.1f | %10.1f | %7.2fx | %.0f-%.0f\n",
            "find", rb_s.avg, std_s.avg, std_s.avg/rb_s.avg, rb_s.min, rb_s.max);
        fflush(stdout);
    }
    
    // === Erase ===
    {
        rb_times.clear(); std_times.clear();
        std::vector<int> data(N), erasing(N/2);
        for (int r = 0; r < ROUNDS; ++r)
        {
            base_rng.seed(42 + r * 1000);
            for (auto& v : data) v = base_rng();
            Yc::rb_set<int> rb; std::set<int> s;
            for (int v : data) { rb.insert(v); s.insert(v); }
            
            base_rng.seed(99 + r * 1000);
            for (auto& v : erasing) v = base_rng();
            
            auto t0 = high_resolution_clock::now();
            for (int v : erasing) s.erase(v);
            auto t1 = high_resolution_clock::now();
            for (int v : erasing) rb.erase(v);
            auto t2 = high_resolution_clock::now();
            std_times.push_back(to_us(t1 - t0) / 1000.0);
            rb_times.push_back(to_us(t2 - t1) / 1000.0);
        }
        auto rb_s = compute(rb_times), std_s = compute(std_times);
        printf("  %-18s | %10.1f | %10.1f | %7.2fx | %.0f-%.0f\n",
            "erase", rb_s.avg, std_s.avg, std_s.avg/rb_s.avg, rb_s.min, rb_s.max);
        fflush(stdout);
    }
    
    // === Iteration ===
    {
        rb_times.clear(); std_times.clear();
        std::vector<int> data(N);
        for (int r = 0; r < ROUNDS; ++r)
        {
            base_rng.seed(42 + r * 1000);
            for (auto& v : data) v = base_rng();
            Yc::rb_set<int> rb; std::set<int> s;
            for (int v : data) { rb.insert(v); s.insert(v); }
            volatile long long sum = 0;
            
            auto t0 = high_resolution_clock::now();
            for (auto x : s) sum += x;
            auto t1 = high_resolution_clock::now();
            for (auto x : rb) sum += x;
            auto t2 = high_resolution_clock::now();
            (void)sum;
            std_times.push_back(to_us(t1 - t0) / 1000.0);
            rb_times.push_back(to_us(t2 - t1) / 1000.0);
        }
        auto rb_s = compute(rb_times), std_s = compute(std_times);
        printf("  %-18s | %10.1f | %10.1f | %7.2fx | %.0f-%.0f\n",
            "iteration", rb_s.avg, std_s.avg, std_s.avg/rb_s.avg, rb_s.min, rb_s.max);
        fflush(stdout);
    }
    
    // === Mixed ===
    {
        rb_times.clear(); std_times.clear();
        int M = N / 2;
        for (int r = 0; r < ROUNDS; ++r)
        {
            base_rng.seed(42 + r * 1000);
            auto t0 = high_resolution_clock::now();
            {
                std::set<int> st;
                for (int i = 0; i < M; ++i) st.insert(base_rng());
                volatile long long ss = 0;
                for (int i = 0; i < M/2; ++i) if (st.find(base_rng()) != st.end()) ss += 1;
                for (int i = 0; i < M/3; ++i) st.erase(base_rng());
                for (auto x : st) ss += x;
                (void)ss;
            }
            auto t1 = high_resolution_clock::now();
            
            base_rng.seed(42 + r * 1000);
            {
                Yc::rb_set<int> rb;
                for (int i = 0; i < M; ++i) rb.insert(base_rng());
                volatile long long ss = 0;
                for (int i = 0; i < M/2; ++i) if (rb.find(base_rng()) != rb.end()) ss += 1;
                for (int i = 0; i < M/3; ++i) rb.erase(base_rng());
                for (auto x : rb) ss += x;
                (void)ss;
            }
            auto t2 = high_resolution_clock::now();
            std_times.push_back(to_us(t1 - t0) / 1000.0);
            rb_times.push_back(to_us(t2 - t1) / 1000.0);
        }
        auto rb_s = compute(rb_times), std_s = compute(std_times);
        printf("  %-18s | %10.1f | %10.1f | %7.2fx | %.0f-%.0f\n",
            "mixed", rb_s.avg, std_s.avg, std_s.avg/rb_s.avg, rb_s.min, rb_s.max);
        fflush(stdout);
    }
    
    printf("\n  Ratio > 1.00 = Yc::rb_set faster.  Range = min-max of %d rounds.\n", ROUNDS);
    fflush(stdout);
    return 0;
}
