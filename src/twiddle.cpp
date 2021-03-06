#include "twiddle.h"

#include <fstream>
#include <iostream>

Twiddle::Twiddle(PID& pid, const std::vector<double>& p, const std::vector<double>& dp, std::ostream& os)
  : m_pid(pid),
    m_p(p),
    m_dp(dp),
    m_best_err(std::numeric_limits<double>::max()),
    m_best_run(0),
    m_run(0),
    m_iteration(0),
    m_p_index(0),
    m_tried_negative(false),
    m_os(os)
{
  m_stopwatch.Start();
}

void Twiddle::AdvanceParameter()
{
  m_p_index = (m_p_index + 1);
  if (m_p_index == 3)
  {
    m_p_index = 0;
    m_iteration++;
  }
  m_tried_negative = false;

  // Try positive direction.
  m_p[m_p_index] += m_dp[m_p_index];

  m_pid.Init(m_p[0], m_p[1], m_p[2]);
}

void Twiddle::PrintStatus(std::ostream& os, double err, double seconds)
{
    os << "run=[" << m_run
        << "] err=[" << err
        << "] seconds=[" << seconds
        << "] kP=[" << m_p[0]
        << "] kI=[" << m_p[1]
        << "] kD=[" << m_p[2]
        << "] iteration=[" << m_iteration
        << "] best_run=[" << m_best_run
        << "] best_err=[" << m_best_err
        << "]" << std::endl;
}

void Twiddle::NextRun()
{
  m_stopwatch.Stop();

  double run_time = m_stopwatch.GetElapsedSeconds();

  const double err = (m_pid.TotalError() / run_time) + (2000.0 / run_time);

  PrintStatus(std::cout, err, run_time);
  PrintStatus(m_os, err, run_time);

  m_stopwatch.Reset();
  m_stopwatch.Start();

  if (err < m_best_err)
  {
    m_best_err = err;
    m_best_run = m_run;
    m_dp[m_p_index] *= 1.1;

    AdvanceParameter();
  }
  else
  {
    if (m_tried_negative)
    {
      m_p[m_p_index] += m_dp[m_p_index];
      m_dp[m_p_index] *= 0.9;

      AdvanceParameter();
    }
    else
    {
      m_p[m_p_index] -= 2 * m_dp[m_p_index];
      m_tried_negative = true;

      m_pid.Init(m_p[0], m_p[1], m_p[2]);
    }
  }

  m_run++;
}
