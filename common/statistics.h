#pragma once

#include <cmath>

#include <boost/type_traits/is_assignable.hpp>
#include <boost/circular_buffer.hpp>

namespace pt
{

template <typename T>
T stdev(T cnt, T sum, T ssq)
{
    return std::sqrt((cnt * ssq - std::pow(sum, T(2))) / (cnt * (cnt - 1)));
}

template <typename Scalar>
struct MovingAvg
{
    explicit MovingAvg(int n) : sum_(0), ssq_(0)
    {
        q_ = boost::circular_buffer<Scalar>(n);
    }
    void clear()
    {
        q_.clear();
    }
    void push(Scalar v)
    {
        if (q_.size() == q_.capacity())
        {
            Scalar t = q_.front();
            sum_ -= t;
            ssq_ -= t * t;
            q_.pop_front();
        }
        q_.push_back(v);
        sum_ += v;
        ssq_ += v * v;
    }
    Scalar sum() const
    {
        return sum_;
    }
    std::size_t size() const
    {
        return q_.size();
    }
    Scalar mean() const
    {
        const std::size_t s = size();
        return s > 0 ? sum_ / float(s) : Scalar();
    }
    Scalar stdev() const
    {
        return pt::stdev(size(), sum_, ssq_);
    }

private:
    boost::circular_buffer<Scalar> q_;
    Scalar sum_;
    Scalar ssq_;
};

} // namespace pt
