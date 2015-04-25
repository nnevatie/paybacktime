#pragma once

#include <cmath>
#include <boost/circular_buffer.hpp>

namespace hc
{

template <typename T>
T stdev(T cnt, T sum, T ssq)
{
    return std::sqrt((cnt * ssq - std::pow(sum, T(2))) / (cnt * (cnt - 1)));
}

template <typename Scalar>
struct MovingAvg
{
    MovingAvg(int n) : sum(0), ssq(0)
    {
        q = boost::circular_buffer<int>(n);
    }
    void push(Scalar v)
    {
        if (q.size() == q.capacity())
        {
            Scalar t = q.front();
            sum -= t;
            ssq -= t * t;
            q.pop_front();
        }
        q.push_back(v);
        sum += v;
        ssq += v * v;
    }
    Scalar size()
    {
        return q.size();
    }
    Scalar mean()
    {
        return sum / size();
    }
    Scalar stdev()
    {
        return hc::stdev(size(), sum, ssq);
    }

private:
    boost::circular_buffer<int> q;
    Scalar sum;
    Scalar ssq;
};

} // namespace hc
