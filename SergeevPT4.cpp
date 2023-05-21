#include <iostream>
#include <ctime>
#include <fstream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <string>
#include <map>
#include <deque>
#include <random>

#define uint unsigned long long

using namespace std;

string inputFile = "data.txt";
string outputFile = "output.txt";

ofstream fout(outputFile);

const uint sample1 = 101;
const uint samples = 10;
const uint maximum = 1 << 20;

const int Num = 6;
const int arr[Num] = { 1000, 5000, 10000, 50000, 100000, 1000000 };

class Mersenne_Twister {
    std::mt19937 mt;

public:
    void setSeed(uint seed) {
        mt.seed(seed);
    }

    uint getNumber() {
        return mt() % maximum;
    }

    vector<uint> getVector(size_t size) {
        vector<uint> v;
        v.resize(size);

        for (size_t i = 0; i < size; ++i)
            v[i] = this->getNumber();

        return v;
    }
};

class XORShift {
    uint seed;

public:
    void setSeed(uint seed) {
        this->seed = seed;
    }

    uint getNumber() {
        seed ^= (seed << 13);
        seed ^= (seed >> 17);
        seed ^= (seed << 5);
        return seed % maximum;
    }

    vector<uint> getVector(size_t size) {
        vector<uint> v;
        v.resize(size);

        for (size_t i = 0; i < size; ++i)
            v[i] = this->getNumber();

        return v;
    }
};


double mean(vector<uint>& v) {
    unsigned long long sum = 0;

    for (auto& n : v)
        sum += n;

    return sum / static_cast<double>(v.size());
}

double deviation(double mean, vector<uint>& v) {
    double sum = 0;

    for (auto& n : v)
        sum += (n - mean) * (n - mean);

    return sqrt(sum / v.size());
}

double variationCoeff(double deviation, double mean) {
    return deviation / mean;
}

double chiSquare(vector<uint>& vec) {
    //считается, что в v лежат целые числа, до maximum, поэтому их надо нормировать
    vector<double> v;
    for (int i = 0; i < vec.size(); ++i)
        v.push_back(vec[i] / static_cast<double>(maximum - 1));

    const unsigned int n = v.size();
    const unsigned int k = 1 + 3.322 * log(n);
    const double p = 1 / static_cast<double>(k);

    vector<unsigned int> n_i(k, 0);

    for (auto& u : v)
        for (int j = 0; j < k; ++j)
            if (p * j <= u && p * (j + 1) > u)
                n_i[j] += 1;

    double chi = 0;

    //суммирование
    for (auto& u : n_i)
        chi += ((static_cast<double>(u) - p * n) * (static_cast<double>(u) - p * n)) / (p * n);

    return chi;
}

void writeTime(string title, std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point end, int divideBy = 1) {
    fout << title;

    fout << chrono::duration_cast<chrono::microseconds>(end - start).count() / divideBy << " [микросекунд]\n";
}

void writeSampleInfo(vector<uint>& vec) {
    double m = mean(vec);
    double d = deviation(m, vec);
    double v = variationCoeff(d, m);
    double chi = chiSquare(vec);

    fout << "Среднее: " << m << "\nОтклонение: " << d << "\nКоэффициент вариации: " << v
        << "\nКритерий хи-квадрат: " << chi << "\n\n";
}

int main()
{
    setlocale(LC_ALL, "Russian");

    std::chrono::steady_clock::time_point start, end;

    Mersenne_Twister eng;

    fout << "Первый генератор:\n" << '\n';

    eng.setSeed(time(NULL));

    for (int i = 0; i < samples; ++i) {
        vector<uint> v = eng.getVector(sample1);
        writeSampleInfo(v);
    }

    start = std::chrono::steady_clock::now();

    for (int i = 0; i < Num; ++i) {
        //генерация с засечением времени
        for (int j = 0; j < arr[i]; ++j)
            const uint num = eng.getNumber();

        end = std::chrono::steady_clock::now();
        writeTime("На генерацию " + to_string(arr[i]) + " значений ушло : ", start, end);
        start = end;
    }

    XORShift eng1;

    eng1.setSeed(time(NULL));

    fout << "\nВторой генератор:\n" << '\n';

    for (int i = 0; i < samples; ++i) {
        vector<uint> v = eng1.getVector(sample1);
        writeSampleInfo(v);
    }

    start = std::chrono::steady_clock::now();

    for (int i = 0; i < Num; ++i) {
        //генерация с засечением времени
        for (int j = 0; j < arr[i]; ++j)
            const uint num = eng1.getNumber();

        end = std::chrono::steady_clock::now();
        writeTime("На генерацию " + to_string(arr[i]) + " значений ушло : ", start, end);
        start = end;
    }

    fout << "\nСравнение с mt19937:\n" << '\n';

    mt19937 mt_rand(time(0));
    for (int l = 0; l < samples; ++l) {
        vector<uint> vRand(sample1, 0);
        for (int i = 0; i < sample1; ++i)
            vRand[i] = mt_rand() % maximum;
        writeSampleInfo(vRand);
    }

    start = std::chrono::steady_clock::now();

    for (int i = 0; i < Num; ++i) {
        //генерация с засечением времени
        for (int j = 0; j < arr[i]; ++j)
            const uint num = mt_rand();

        end = std::chrono::steady_clock::now();
        writeTime("На генерацию " + to_string(arr[i]) + " значений ушло : ", start, end);
        start = end;
    }


    return 0;
}
