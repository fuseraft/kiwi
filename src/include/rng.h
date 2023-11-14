#ifndef NOCTIS_RNG_H
#define NOCTIS_RNG_H

class RNG
{
public:
    static void seed();
    static double random(double min, double max);
    static string random(string start, string sc);
};

void RNG::seed()
{
    srand((unsigned int)time(NULL));
}

double RNG::random(double min, double max)
{
    double r = (double)rand() / (double)RAND_MAX;
    return min + (r * (max - min));
}

string RNG::random(string start, string sc)
{
    string s("");
    char c;
    c = start[0] == sc[0] ? sc[0] : (rand() % get_alpha_num(sc[0])) + start[0];
    s.push_back(c);

    return s;
}

#endif