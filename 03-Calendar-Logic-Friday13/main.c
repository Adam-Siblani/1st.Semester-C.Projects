#ifndef __PROGTEST__
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

typedef struct TDate
{
  unsigned       m_Year;
  unsigned short m_Month;
  unsigned short m_Day;
} TDATE;

TDATE makeDate ( unsigned y, unsigned short m, unsigned short d )
{
  TDATE res = { y, m, d };
  return res;
}

bool equalDate ( TDATE a, TDATE b )
{
  return a . m_Year == b . m_Year
      && a . m_Month == b . m_Month
      && a . m_Day == b . m_Day;
}
#endif /* __PROGTEST__ */

#define BASE_YEAR 1900

/**
 * Custom Leap Year Calculation
 * Rules: Divisible by 4, except multiples of 100, 
 * but including multiples of 400, and excluding multiples of 4000.
 */
static bool leapCalc ( long long yr )
{
  if ( yr % 4000 == 0 ) return false;
  if ( yr % 400 == 0 ) return true;
  if ( yr % 100 == 0 ) return false;
  return ( yr % 4 == 0 );
}

/**
 * Returns the number of days in a specific month for a specific year.
 */
static int monthDays ( long long yr, int mon )
{
  static const int md[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
  if ( mon == 2 && leapCalc(yr) ) return 29;
  return md[mon - 1];
}

/**
 * Validates if a TDATE structure represents a real calendar date.
 */
static bool legitDate ( TDATE d )
{
  if ( d.m_Year < BASE_YEAR || d.m_Month < 1 || d.m_Month > 12 ) return false;
  if ( d.m_Day < 1 || d.m_Day > monthDays(d.m_Year, d.m_Month) ) return false;
  return true;
}

/**
 * Calculates total days elapsed from Jan 1st, 1900 to the given date.
 * Used for comparing which of two dates comes first.
 */
static long long dayCountFrom1900 ( TDATE d )
{
  long long total = 0;
  for ( long long y = BASE_YEAR; y < d.m_Year; ++y )
    total += leapCalc(y) ? 366 : 365;
  for ( int m = 1; m < d.m_Month; ++m )
    total += monthDays(d.m_Year, m);
  total += d.m_Day - 1;
  return total;
}

/**
 * Zeller’s Congruence Algorithm
 * Returns the day of the week (0=Sunday, ..., 5=Friday, 6=Saturday).
 */
static int weekDay ( TDATE d )
{
  int y = d.m_Year;
  int m = d.m_Month;
  if ( m < 3 ) { m += 12; y -= 1; }
  int k = y % 100;
  int j = y / 100;
  int h = ( d.m_Day + (13*(m + 1))/5 + k + k/4 + j/4 + 5*j ) % 7;
  int w = ((h + 6) % 7); 
  return w;
}

/**
 * Helper to check if a specific date is both valid and a Friday the 13th.
 */
static bool isFriday13 ( TDATE d )
{
  return legitDate(d) && d.m_Day == 13 && weekDay(d) == 5;
}

/**
 * Counts occurrences of Friday 13th between two inclusive dates.
 */
bool countFriday13 ( TDATE from, TDATE to, long long int * cnt )
{
  if ( !cnt || !legitDate(from) || !legitDate(to) ) return false;
  if ( dayCountFrom1900(to) < dayCountFrom1900(from) ) return false;
  
  long long res = 0;
  long long y1 = from.m_Year, y2 = to.m_Year;
  int startM = from.m_Month, endM = to.m_Month;

  for ( long long y = y1; y <= y2; ++y )
  {
    int mStart = ( y == y1 ? startM : 1 );
    int mEnd = ( y == y2 ? endM : 12 );
    for ( int m = mStart; m <= mEnd; ++m )
    {
      TDATE chk = makeDate(y, m, 13);
      if ( !legitDate(chk) ) continue;
      if ( dayCountFrom1900(chk) < dayCountFrom1900(from) ||
           dayCountFrom1900(chk) > dayCountFrom1900(to) )
        continue;
      if ( isFriday13(chk) ) ++res;
    }
  }
  *cnt = res;
  return true;
}

/**
 * Modifies the provided date to the nearest preceding Friday 13th.
 */
bool prevFriday13 ( TDATE * date )
{
  if ( !date || !legitDate(*date) ) return false;
  int y = date->m_Year;
  int m = date->m_Month;
  while (1)
  {
    TDATE cand = makeDate(y, m, 13);
    if ( legitDate(cand) && dayCountFrom1900(cand) < dayCountFrom1900(*date) && isFriday13(cand) )
    {
      *date = cand;
      return true;
    }
    if (--m < 1) { m = 12; y--; }
    if ( y < BASE_YEAR ) return false;
  }
  return false;
}

/**
 * Modifies the provided date to the nearest succeeding Friday 13th.
 */
bool nextFriday13 ( TDATE * date )
{
  if ( !date || !legitDate(*date) ) return false;
  int y = date->m_Year;
  int m = date->m_Month;
  while (1)
  {
    TDATE cand = makeDate(y, m, 13);
    if ( legitDate(cand) && dayCountFrom1900(cand) > dayCountFrom1900(*date) && isFriday13(cand) )
    {
      *date = cand;
      return true;
    }
    if (++m > 12) { m = 1; y++; }
  }
  return false;
}

#ifndef __PROGTEST__
/* Example main and testing logic would go here */
#endif
