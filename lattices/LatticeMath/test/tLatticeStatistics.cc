//# tLatticeStatistics.cc: test LatticeStatistics class
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
// 
#include <casacore/casa/aips.h>
#include <casacore/casa/Arrays/Array.h>
#include <casacore/casa/Arrays/ArrayMath.h>
#include <casacore/casa/Exceptions/Error.h>
#include <casacore/casa/Inputs/Input.h>
#include <casacore/casa/Logging.h>
#include <casacore/scimath/Mathematics/NumericTraits.h>
#include <casacore/casa/BasicSL/String.h>
#include <casacore/casa/Utilities/Regex.h>
#include <casacore/lattices/Lattices/ArrayLattice.h>
#include <casacore/lattices/LatticeMath/LatticeStatistics.h>
#include <casacore/lattices/Lattices/SubLattice.h>
#include <casacore/lattices/LatticeMath/LatticeStatsBase.h>
#include <casacore/lattices/Lattices/LatticeUtilities.h>
#include <casacore/lattices/LRegions/LCSlicer.h>
#include <casacore/scimath/StatsFramework/ClassicalStatistics.h>

#include <casacore/casa/iostream.h>

#include <casacore/casa/namespace.h>

void doitFloat(LogIO& os);
void do1DFloat (const Vector<Float>& results,
                const Vector<Bool>& hasResult, 
                const Array<Float>& inArr,
                LogIO& os);
void do2DFloat (const Vector<Float>& results,
                const Vector<Bool>& hasResult, 
                const Array<Float>& inArr,
                LogIO& os);
void test1DFloat (LatticeStatistics<Float>& stats, const Vector<Float>& results,
                  const Vector<Bool>& hasResult, const IPosition& shape);
void test2DFloat (LatticeStatistics<Float>& stats, const Vector<Float>& results,
                  const Vector<Bool>& hasResult, const IPosition& shape);


int main()
{
    try {
        LogOrigin lor("tLatticeStatistics", "main()", WHERE);
        LogIO os(lor);
        doitFloat(os);

        Vector<Float> data(1000);
        Vector<Float>::iterator iter = data.begin();
        Vector<Float>::iterator end = data.end();
        uInt count = 0;
        while(iter != end) {
            *iter = count % 2 == 0 ? (Float)count : -(Float)(count*count);
            ++iter;
            ++count;
        }
        {
            ArrayLattice<Float> latt(data);
            SubLattice<Float> subLatt(latt);
            LatticeStatistics<Float> stats(subLatt);
            Array<Double> median, iqr, medabsdevmed, npts, q1, q3;
            stats.getStatistic(median, LatticeStatsBase::MEDIAN, False);
            AlwaysAssert(*median.begin() == -0.5, AipsError);
            stats.getStatistic(q1, LatticeStatsBase::Q1, False);
            AlwaysAssert(*q1.begin() == -251001, AipsError);
            stats.getStatistic(q3, LatticeStatsBase::Q3, False);
            AlwaysAssert(*q3.begin() == 498, AipsError);
            Vector<Float> range(2, 0.1);
            range[1] = 1001;
            stats.setInExCludeRange(range, Vector<Float>(), False);
            stats.getStatistic(median, LatticeStatsBase::MEDIAN, False);
            AlwaysAssert(*median.begin() == 500, AipsError);
            stats.getStatistic(iqr, LatticeStatsBase::QUARTILE, False);
            AlwaysAssert(*iqr.begin() == 500, AipsError);
            stats.getStatistic(medabsdevmed, LatticeStatsBase::MEDABSDEVMED, False);
            AlwaysAssert(*medabsdevmed.begin() == 250, AipsError);
            stats.getStatistic(q1, LatticeStatsBase::Q1, False);
            AlwaysAssert(*q1.begin() == 250, AipsError);
            stats.getStatistic(q3, LatticeStatsBase::Q3, False);
            AlwaysAssert(*q3.begin() == 750, AipsError);

            // exclude range
            stats.setInExCludeRange(Vector<Float>(), range, False);
            stats.getStatistic(median, LatticeStatsBase::MEDIAN, False);
            AlwaysAssert(*median.begin() == -249001, AipsError);
            stats.getStatistic(iqr, LatticeStatsBase::QUARTILE, False);
            AlwaysAssert(*iqr.begin() == 499000, AipsError);
            stats.getStatistic(medabsdevmed, LatticeStatsBase::MEDABSDEVMED, False);
            AlwaysAssert(*medabsdevmed.begin() == 216240, AipsError);
            stats.getStatistic(q1, LatticeStatsBase::Q1, False);
            AlwaysAssert(*q1.begin() == -561001, AipsError);
            stats.getStatistic(q3, LatticeStatsBase::Q3, False);
            AlwaysAssert(*q3.begin() == -62001, AipsError);

            // mask
            Vector<Bool> mask(1000);
            Vector<Bool>::iterator miter = mask.begin();
            Vector<Bool>::iterator mend = mask.end();
            count = 0;
            while (miter != mend) {
                *miter = count % 3 == 0;
                ++miter;
                ++count;
            }
            subLatt.setPixelMask(ArrayLattice<Bool>(mask), True);
            stats = LatticeStatistics<Float>(subLatt);
            stats.getStatistic(npts, LatticeStatsBase::NPTS, False);
            AlwaysAssert(*npts.begin() == 334, AipsError);
            stats.getStatistic(median, LatticeStatsBase::MEDIAN, False);
            AlwaysAssert(*median.begin() == -4.5, AipsError);
            stats.getStatistic(q1, LatticeStatsBase::Q1, False);
            AlwaysAssert(*q1.begin() == -251001, AipsError);
            stats.getStatistic(q3, LatticeStatsBase::Q3, False);
            AlwaysAssert(*q3.begin() == 498, AipsError);

            // include range
            stats.setInExCludeRange(range, Vector<Float>(), False);
            stats.getStatistic(median, LatticeStatsBase::MEDIAN, False);
            AlwaysAssert(*median.begin() == 501, AipsError);
            stats.getStatistic(iqr, LatticeStatsBase::QUARTILE, False);
            AlwaysAssert(*iqr.begin() == 498, AipsError);
            stats.getStatistic(medabsdevmed, LatticeStatsBase::MEDABSDEVMED, False);
            AlwaysAssert(*medabsdevmed.begin() == 249, AipsError);
            stats.getStatistic(q1, LatticeStatsBase::Q1, False);
            AlwaysAssert(*q1.begin() == 252, AipsError);
            stats.getStatistic(q3, LatticeStatsBase::Q3, False);
            AlwaysAssert(*q3.begin() == 750, AipsError);

            // exclude range
            stats.setInExCludeRange(Vector<Float>(), range, False);
            stats.getStatistic(npts, LatticeStatsBase::NPTS, False);
            AlwaysAssert(*npts.begin() == 168, AipsError);
            stats.getStatistic(median, LatticeStatsBase::MEDIAN, False);
            AlwaysAssert(*median.begin() == -248013, AipsError);
            stats.getStatistic(iqr, LatticeStatsBase::QUARTILE, False);
            AlwaysAssert(*iqr.begin() == 505008, AipsError);
            stats.getStatistic(medabsdevmed, LatticeStatsBase::MEDABSDEVMED, False);
            AlwaysAssert(*medabsdevmed.begin() == 216216, AipsError);
            stats.getStatistic(q1, LatticeStatsBase::Q1, False);
            AlwaysAssert(*q1.begin() == -567009, AipsError);
            stats.getStatistic(q3, LatticeStatsBase::Q3, False);
            AlwaysAssert(*q3.begin() == -62001, AipsError);

            // corner case when lattice is completely masked
            mask.set(False);
            subLatt.setPixelMask(ArrayLattice<Bool>(mask), True);
            stats = LatticeStatistics<Float>(subLatt);
            stats.getStatistic(npts, LatticeStatsBase::NPTS, False);
            AlwaysAssert(npts.size() == 0, AipsError);
        }
        {
            // using configure*() methods
            ArrayLattice<Float> latt(data);
            SubLattice<Float> subLatt(latt);
            LatticeStatistics<Float> stats(subLatt);
            stats.configureClassical();
            Array<Double> mean;
            Float expec = casacore::mean(data);
            stats.getStatistic(mean, LatticeStatsBase::MEAN, False);
            AlwaysAssert(near(*mean.begin(), expec), AipsError);
            stats.getStatistic(mean, LatticeStatsBase::MEAN, False);
            AlwaysAssert(near(*mean.begin(), expec), AipsError);
            Array<Double> v;
            stats.getStatistic(v, LatticeStatsBase::MAX, False);
            AlwaysAssert(near(*v.begin(), 998.0), AipsError);
            //hinges-fences
            stats.configureHingesFences(0.0);
            stats.getStatistic(mean, LatticeStatsBase::MEAN, False);
            expec = -41960.081836;
            AlwaysAssert(near(*mean.begin(), expec), AipsError);

            stats.configureFitToHalf(
                FitToHalfStatisticsData::CMEAN,
                FitToHalfStatisticsData::LE_CENTER
            );
            stats.getStatistic(v, LatticeStatsBase::MEAN, False);
            Double m = *v.begin();
            AlwaysAssert(near(m, casacore::mean(data)), AipsError);
            stats.getStatistic(v, LatticeStatsBase::MEDIAN, False);
            AlwaysAssert(near(*v.begin(), m), AipsError);
            stats.getStatistic(v, LatticeStatsBase::NPTS, False);
            Int npts = (Int)*v.begin();
            AlwaysAssert(npts == 592, AipsError);
            stats.getStatistic(v, LatticeStatsBase::SUM, False);
            Double sum = *v.begin();
            AlwaysAssert(near(sum, m*npts), AipsError);
            stats.getStatistic(v, LatticeStatsBase::SUMSQ, False);
            AlwaysAssert(near(*v.begin(), 127119111260752.0), AipsError);
            stats.getStatistic(v, LatticeStatsBase::MIN, False);
            AlwaysAssert(near(*v.begin(), casacore::min(data)), AipsError);
            stats.getStatistic(v, LatticeStatsBase::MAX, False);
            AlwaysAssert(near(*v.begin(), 2*m - casacore::min(data)), AipsError);
            IPosition minPos, maxPos;
            stats.getMinMaxPos(minPos, maxPos);
            AlwaysAssert(minPos.size() == 1 && minPos[0] == 999, AipsError);
            AlwaysAssert(maxPos.size() == 0, AipsError);
            stats.getStatistic(v, LatticeStatsBase::Q1, False);
            AlwaysAssert(near(*v.begin(), -497025.0), AipsError);
            stats.getStatistic(v, LatticeStatsBase::Q3, False);
            AlwaysAssert(near(*v.begin(), 161375.0), AipsError);

            stats.configureFitToHalf(
                FitToHalfStatisticsData::CMEAN,
                FitToHalfStatisticsData::GE_CENTER
            );
            stats.getStatistic(v, LatticeStatsBase::MEAN, False);
            m = *v.begin();
            AlwaysAssert(near(m, casacore::mean(data)), AipsError);
            stats.getStatistic(v, LatticeStatsBase::MEDIAN, False);
            AlwaysAssert(near(*v.begin(), m), AipsError);
            stats.getStatistic(v, LatticeStatsBase::NPTS, False);
            npts = (Int)*v.begin();
            AlwaysAssert(npts == 1408, AipsError);
            stats.getStatistic(v, LatticeStatsBase::SUM, False);
            sum = *v.begin();
            AlwaysAssert(near(sum, m*npts), AipsError);
            stats.getStatistic(v, LatticeStatsBase::SUMSQ, False);
            AlwaysAssert(near(*v.begin(), 72880554407048.0), AipsError);
            stats.getStatistic(v, LatticeStatsBase::MIN, False);
            AlwaysAssert(near(*v.begin(), 2*m - casacore::max(data)), AipsError);
            stats.getStatistic(v, LatticeStatsBase::MAX, False);
            AlwaysAssert(near(*v.begin(), casacore::max(data)), AipsError);
            stats.getMinMaxPos(minPos, maxPos);
            AlwaysAssert(minPos.size() == 0, AipsError);
            AlwaysAssert(maxPos.size() == 1 && maxPos == 998, AipsError);

            stats.configureFitToHalf(
                FitToHalfStatisticsData::CMEDIAN,
                FitToHalfStatisticsData::LE_CENTER
            );
            stats.getStatistic(v, LatticeStatsBase::MEAN, False);
            m = *v.begin();
            AlwaysAssert(near(m, -0.5), AipsError);
            stats.getStatistic(v, LatticeStatsBase::NPTS, False);
            npts = (Int)*v.begin();
            AlwaysAssert(npts == 1000, AipsError);
            stats.getStatistic(v, LatticeStatsBase::SUM, False);
            sum = *v.begin();
            AlwaysAssert(near(sum, m*npts), AipsError);
            stats.getStatistic(v, LatticeStatsBase::SUMSQ, False);
            AlwaysAssert(near(*v.begin(), 199999000001300.0), AipsError);
            stats.getStatistic(v, LatticeStatsBase::MIN, False);
            AlwaysAssert(near(*v.begin(), casacore::min(data)), AipsError);
            stats.getStatistic(v, LatticeStatsBase::MAX, False);
            AlwaysAssert(near(*v.begin(),2*m - casacore::min(data)), AipsError);
            stats.getMinMaxPos(minPos, maxPos);
            AlwaysAssert(minPos.size() == 1 && minPos[0] == 999, AipsError);
            AlwaysAssert(maxPos.size() == 0, AipsError);

            stats.configureFitToHalf(
                FitToHalfStatisticsData::CMEDIAN,
                FitToHalfStatisticsData::GE_CENTER
            );
            stats.getStatistic(v, LatticeStatsBase::MEAN, False);
            m = *v.begin();
            AlwaysAssert(near(m, -0.5), AipsError);
            stats.getStatistic(v, LatticeStatsBase::NPTS, False);
            npts = (Int)*v.begin();
            AlwaysAssert(npts == 1000, AipsError);
            stats.getStatistic(v, LatticeStatsBase::SUM, False);
            sum = *v.begin();
            AlwaysAssert(near(sum, m*npts), AipsError);
            stats.getStatistic(v, LatticeStatsBase::SUMSQ, False);
            AlwaysAssert(near(*v.begin(), 332833500.0), AipsError);
            stats.getStatistic(v, LatticeStatsBase::MIN, False);
            AlwaysAssert(near(*v.begin(), 2*m - casacore::max(data)), AipsError);
            stats.getStatistic(v, LatticeStatsBase::MAX, False);
            AlwaysAssert(near(*v.begin(), casacore::max(data)), AipsError);
            stats.getMinMaxPos(minPos, maxPos);
            AlwaysAssert(minPos.size() == 0, AipsError);
            AlwaysAssert(maxPos.size() == 1 && maxPos[0] == 998, AipsError);

            stats.configureFitToHalf(
                FitToHalfStatisticsData::CVALUE,
                FitToHalfStatisticsData::LE_CENTER,
                65
            );
            stats.getStatistic(v, LatticeStatsBase::MEAN, False);
            m = *v.begin();
            AlwaysAssert(m == 65, AipsError);
            stats.getStatistic(v, LatticeStatsBase::NPTS, False);
            npts = (Int)*v.begin();
            AlwaysAssert(npts == 1066, AipsError);
            stats.getStatistic(v, LatticeStatsBase::SUM, False);
            sum = *v.begin();
            AlwaysAssert(near(sum, m*npts), AipsError);
            stats.getStatistic(v, LatticeStatsBase::SUMSQ, False);
            AlwaysAssert(near(*v.begin(), 200042675448460.0), AipsError);
            stats.getStatistic(v, LatticeStatsBase::MIN, False);
            AlwaysAssert(near(*v.begin(), min(data)), AipsError);
            stats.getStatistic(v, LatticeStatsBase::MAX, False);
            AlwaysAssert(near(*v.begin(), 2*m - casacore::min(data)), AipsError);
            stats.getMinMaxPos(minPos, maxPos);
            AlwaysAssert(minPos.size() == 1 && minPos[0] == 999, AipsError);
            AlwaysAssert(maxPos.size() == 0, AipsError);

            stats.configureFitToHalf(
                FitToHalfStatisticsData::CVALUE,
                FitToHalfStatisticsData::GE_CENTER,
                65
            );
            stats.getStatistic(v, LatticeStatsBase::MEAN, False);
            m = *v.begin();
            AlwaysAssert(m == 65, AipsError);
            stats.getStatistic(v, LatticeStatsBase::NPTS, False);
            npts = (Int)*v.begin();
            AlwaysAssert(npts == 934, AipsError);
            stats.getStatistic(v, LatticeStatsBase::SUM, False);
            sum = *v.begin();
            AlwaysAssert(near(sum, m*npts), AipsError);
            stats.getStatistic(v, LatticeStatsBase::SUMSQ, False);
            AlwaysAssert(near(*v.begin(), 275539340.0), AipsError);
            stats.getStatistic(v, LatticeStatsBase::MIN, False);
            AlwaysAssert(near(*v.begin(), 2*m - max(data)), AipsError);
            stats.getStatistic(v, LatticeStatsBase::MAX, False);
            AlwaysAssert(near(*v.begin(), casacore::max(data)), AipsError);
            stats.getMinMaxPos(minPos, maxPos);
            AlwaysAssert(minPos.size() == 0, AipsError);
            AlwaysAssert(maxPos.size() == 1 && maxPos[0] == 998, AipsError);

            // mask
            Vector<Bool> mask(1000);
            Vector<Bool>::iterator miter = mask.begin();
            Vector<Bool>::iterator mend = mask.end();
            count = 0;
            while (miter != mend) {
                *miter = count % 3 == 0;
                ++miter;
                ++count;
            }
            subLatt.setPixelMask(ArrayLattice<Bool>(mask), True);
            stats = LatticeStatistics<Float>(subLatt);
            stats.configureFitToHalf(
                FitToHalfStatisticsData::CMEAN,
                FitToHalfStatisticsData::LE_CENTER
            );
            stats.getStatistic(v, LatticeStatsBase::MEAN, False);
            m = *v.begin();
            AlwaysAssert(near(m, -167083.5), AipsError);
            stats.getStatistic(v, LatticeStatsBase::NPTS, False);
            npts = (Int)*v.begin();
            AlwaysAssert(npts == 198, AipsError);
            stats.getStatistic(v, LatticeStatsBase::SUM, False);
            sum = *v.begin();
            AlwaysAssert(near(sum, m*npts), AipsError);
            stats.getStatistic(v, LatticeStatsBase::SUMSQ, False);
            AlwaysAssert(near(*v.begin(), 42804555931071.0), AipsError);
            stats.getStatistic(v, LatticeStatsBase::MIN, False);
            AlwaysAssert(near(*v.begin(), -998001.0), AipsError);
            stats.getStatistic(v, LatticeStatsBase::MAX, False);
            AlwaysAssert(near(*v.begin(), 2*-167083.5 - -998001.0), AipsError);
            stats.getMinMaxPos(minPos, maxPos);
            AlwaysAssert(minPos.size() == 1 && minPos[0] == 999, AipsError);
            AlwaysAssert(maxPos.size() == 0, AipsError);
            // biweight
            // unset the mask
            mask.set(True);
            subLatt.setPixelMask(ArrayLattice<Bool>(mask), True);
            stats.configureBiweight(20, 6);
            Bool thrown = False;
            try {
                stats.getStatistic(v, LatticeStatsBase::SUM, False);
            }
            catch (const AipsError&) {
                thrown = True;
            }
            AlwaysAssert(thrown, AipsError);
            thrown = False;
            try {
                stats.getStatistic(v, LatticeStatsBase::MEDABSDEVMED, False);
            }
            catch (const AipsError&) {
                thrown = True;
            }
            AlwaysAssert(thrown, AipsError);
            thrown = False;
            try {
                stats.getMinMaxPos(minPos, maxPos);
            }
            catch (const AipsError&) {
                thrown = True;
            }
            AlwaysAssert(thrown, AipsError);
            stats.getStatistic(v, LatticeStatsBase::MAX, False);
            AlwaysAssert(near(*v.begin(), 998.0), AipsError);
            stats.getStatistic(v, LatticeStatsBase::MIN, False);
            AlwaysAssert(near(*v.begin(), -998001.0), AipsError);
            stats.getStatistic(v, LatticeStatsBase::NPTS, False);
            AlwaysAssert(near(*v.begin(), 1000.0), AipsError);
            stats.getStatistic(v, LatticeStatsBase::MEAN, False);
            AlwaysAssert(near(*v.begin(), 471.024223013, 3e-5), AipsError);
            stats.getStatistic(v, LatticeStatsBase::SIGMA, False);
            AlwaysAssert(near(*v.begin(), 461.243958957, 2e-5), AipsError);
        }
        {
            cout << "test stats for complex value lattice using old and new methods" << endl;
            uInt size = 500000;
            Vector<Complex> cdata(size);
            Vector<Complex>::iterator iter = cdata.begin();
            Vector<Complex>::iterator end = cdata.end();
            Float i = 0;
            DComplex expMean((size -1 )/2.0, (size - 1)/2.0);
            DComplex expNVar = 0;
            DComplex expSumSq(0, 0);
            DComplex diff(0, 0);
            for (; iter!=end; ++iter, ++i) {
                *iter = Complex(i, i);
                expSumSq += *iter * *iter;
                diff = *iter - expMean;
                expNVar += diff*diff;
            }
            DComplex expSum(124999750000, 124999750000);
            DComplex expNpts(size, 0);
            DComplex expVar = expNVar/DComplex(size - 1, 0);
            DComplex expSigma = sqrt(expVar);
            DComplex expRMS = sqrt(expSumSq/(Double)size);
            Array<DComplex> sum, npts, mean, sumsq, var,
                sigma, rms, mymax, mymin;
            IPosition pos(1, 0);
            ArrayLattice<Complex> latt(cdata);
            SubLattice<Complex> subLatt(latt);
            LatticeStatistics<Complex> statsOld(subLatt);
            statsOld.configureClassical(0, 0, 1, 1);
            statsOld.getStatistic(sum, LatticeStatsBase::SUM);
            statsOld.getStatistic(npts, LatticeStatsBase::NPTS);
            statsOld.getStatistic(mean, LatticeStatsBase::MEAN);
            statsOld.getStatistic(sumsq, LatticeStatsBase::SUMSQ);
            statsOld.getStatistic(var, LatticeStatsBase::VARIANCE);
            statsOld.getStatistic(sigma, LatticeStatsBase::SIGMA);
            statsOld.getStatistic(rms, LatticeStatsBase::RMS);
            statsOld.getStatistic(mymax, LatticeStatsBase::MAX);
            statsOld.getStatistic(mymin, LatticeStatsBase::MIN);
            AlwaysAssert(sum(pos) == expSum, AipsError);
            AlwaysAssert(npts(pos) == expNpts, AipsError);
            AlwaysAssert(mean(pos) == expMean, AipsError);
            AlwaysAssert(near(sumsq(pos), expSumSq, 1e-9), AipsError);
            AlwaysAssert(near(var(pos), expVar, 1e-10), AipsError);
            AlwaysAssert(near(sigma(pos), expSigma, 1e-11), AipsError);
            AlwaysAssert(near(rms(pos), expRMS, 1e-10), AipsError);
            AlwaysAssert(mymin(pos) == DComplex(0, 0), AipsError);
            AlwaysAssert(mymax(pos) == DComplex(size-1, size-1), AipsError);

            LatticeStatistics<Complex> statsNew(subLatt);
            statsNew.configureClassical(1, 1, 0, 0);
            statsNew.getStatistic(sum, LatticeStatsBase::SUM);
            statsNew.getStatistic(npts, LatticeStatsBase::NPTS);
            statsNew.getStatistic(mean, LatticeStatsBase::MEAN);
            statsNew.getStatistic(sumsq, LatticeStatsBase::SUMSQ);
            statsNew.getStatistic(var, LatticeStatsBase::VARIANCE);
            statsNew.getStatistic(sigma, LatticeStatsBase::SIGMA);
            statsNew.getStatistic(rms, LatticeStatsBase::RMS);
            statsNew.getStatistic(mymax, LatticeStatsBase::MAX);
            statsNew.getStatistic(mymin, LatticeStatsBase::MIN);
            AlwaysAssert(sum(pos) == expSum, AipsError);
            AlwaysAssert(npts(pos) == expNpts, AipsError);
            AlwaysAssert(mean(pos) == expMean, AipsError);
            AlwaysAssert(near(sumsq(pos), expSumSq, 1e-9), AipsError);
            AlwaysAssert(near(var(pos), expVar, 1e-10), AipsError);
            AlwaysAssert(near(sigma(pos), expSigma, 1e-11), AipsError);
            AlwaysAssert(near(rms(pos), expRMS, 1e-9), AipsError);
            AlwaysAssert(mymin(pos) == DComplex(0, 0), AipsError);
            AlwaysAssert(mymax(pos) == DComplex(size-1, size-1), AipsError);

            Vector<Complex> include(2), exclude(2);
            include[0] = Complex(10000, 10000);
            include[1] = Complex(20000, 20000);
            exclude[0] = Complex(400000, 400000);
            exclude[1] = Complex(600000, 600000);

            // unmasked, include range
            expSum = DComplex(150015000, 150015000);
            expNpts = DComplex(10001, 0);
            expSumSq = DComplex(0, 4667166670000);
            expMean = expSum/expNpts;
            expNVar = DComplex(0, 166716670000);
            expVar = expNVar/(expNpts - 1);
            expSigma = sqrt(expVar);
            expRMS = sqrt(expSumSq/expNpts);
            statsOld.setInExCludeRange(include, Vector<Complex>());
            statsOld.getStatistic(sum, LatticeStatsBase::SUM);
            statsOld.getStatistic(npts, LatticeStatsBase::NPTS);
            statsOld.getStatistic(mean, LatticeStatsBase::MEAN);
            statsOld.getStatistic(sumsq, LatticeStatsBase::SUMSQ);
            statsOld.getStatistic(var, LatticeStatsBase::VARIANCE);
            statsOld.getStatistic(sigma, LatticeStatsBase::SIGMA);
            statsOld.getStatistic(rms, LatticeStatsBase::RMS);
            statsOld.getStatistic(mymax, LatticeStatsBase::MAX);
            statsOld.getStatistic(mymin, LatticeStatsBase::MIN);
            AlwaysAssert(sum(pos) == expSum, AipsError);
            AlwaysAssert(npts(pos) == expNpts, AipsError);
            AlwaysAssert(mean(pos) == expMean, AipsError);
            AlwaysAssert(near(sumsq(pos), expSumSq, 1e-8), AipsError);
            AlwaysAssert(near(var(pos), expVar, 1e-8), AipsError);
            AlwaysAssert(near(sigma(pos), expSigma, 1e-11), AipsError);
            AlwaysAssert(near(rms(pos), expRMS, 1e-8), AipsError);
            AlwaysAssert(mymin(pos) == (DComplex)include[0], AipsError);
            AlwaysAssert(mymax(pos) == (DComplex)include[1], AipsError);
            statsNew.setInExCludeRange(include, Vector<Complex>());
            statsNew.getStatistic(sum, LatticeStatsBase::SUM);
            statsNew.getStatistic(npts, LatticeStatsBase::NPTS);
            statsNew.getStatistic(mean, LatticeStatsBase::MEAN);
            statsNew.getStatistic(sumsq, LatticeStatsBase::SUMSQ);
            statsNew.getStatistic(var, LatticeStatsBase::VARIANCE);
            statsNew.getStatistic(sigma, LatticeStatsBase::SIGMA);
            statsNew.getStatistic(rms, LatticeStatsBase::RMS);
            statsNew.getStatistic(mymax, LatticeStatsBase::MAX);
            statsNew.getStatistic(mymin, LatticeStatsBase::MIN);
            AlwaysAssert(sum(pos) == expSum, AipsError);
            AlwaysAssert(npts(pos) == expNpts, AipsError);
            AlwaysAssert(mean(pos) == expMean, AipsError);
            AlwaysAssert(near(sumsq(pos), expSumSq, 1e-8), AipsError);
            AlwaysAssert(near(var(pos), expVar, 1e-8), AipsError);
            AlwaysAssert(near(sigma(pos), expSigma, 1e-11), AipsError);
            AlwaysAssert(near(rms(pos), expRMS, 1e-8), AipsError);
            AlwaysAssert(mymin(pos) == (DComplex)include[0], AipsError);
            AlwaysAssert(mymax(pos) == (DComplex)include[1], AipsError);

            // unmasked, exclude range
            expSum = DComplex(79999800000, 79999800000);
            expNpts = DComplex(400000, 0);
            expSumSq = DComplex(0, 42666506666700080);
            expMean = expSum/expNpts;
            expNVar = DComplex(0, 10666666666446218);
            expVar = expNVar/(expNpts - 1);
            expSigma = sqrt(expVar);
            expRMS = sqrt(expSumSq/expNpts);
            statsOld.setInExCludeRange(Vector<Complex>(), exclude);
            statsOld.getStatistic(sum, LatticeStatsBase::SUM);
            statsOld.getStatistic(npts, LatticeStatsBase::NPTS);
            statsOld.getStatistic(mean, LatticeStatsBase::MEAN);
            statsOld.getStatistic(sumsq, LatticeStatsBase::SUMSQ);
            statsOld.getStatistic(var, LatticeStatsBase::VARIANCE);
            statsOld.getStatistic(sigma, LatticeStatsBase::SIGMA);
            statsOld.getStatistic(rms, LatticeStatsBase::RMS);
            statsOld.getStatistic(mymax, LatticeStatsBase::MAX);
            statsOld.getStatistic(mymin, LatticeStatsBase::MIN);
            AlwaysAssert(sum(pos) == expSum, AipsError);
            AlwaysAssert(npts(pos) == expNpts, AipsError);
            AlwaysAssert(mean(pos) == expMean, AipsError);
            AlwaysAssert(near(sumsq(pos), expSumSq, 1e-8), AipsError);
            AlwaysAssert(near(var(pos), expVar, 1e-8), AipsError);
            AlwaysAssert(near(sigma(pos), expSigma, 1e-11), AipsError);
            AlwaysAssert(near(rms(pos), expRMS, 1e-8), AipsError);
            AlwaysAssert(mymin(pos) == DComplex(0, 0), AipsError);
            AlwaysAssert(mymax(pos) == DComplex(399999, 399999), AipsError);

            statsNew.setInExCludeRange(Vector<Complex>(), exclude);
            statsNew.getStatistic(sum, LatticeStatsBase::SUM);
            statsNew.getStatistic(npts, LatticeStatsBase::NPTS);
            statsNew.getStatistic(mean, LatticeStatsBase::MEAN);
            statsNew.getStatistic(sumsq, LatticeStatsBase::SUMSQ);
            statsNew.getStatistic(var, LatticeStatsBase::VARIANCE);
            statsNew.getStatistic(sigma, LatticeStatsBase::SIGMA);
            statsNew.getStatistic(rms, LatticeStatsBase::RMS);
            statsNew.getStatistic(mymax, LatticeStatsBase::MAX);
            statsNew.getStatistic(mymin, LatticeStatsBase::MIN);
            AlwaysAssert(sum(pos) == expSum, AipsError);
            AlwaysAssert(npts(pos) == expNpts, AipsError);
            AlwaysAssert(mean(pos) == expMean, AipsError);
            AlwaysAssert(near(sumsq(pos), expSumSq, 1e-8), AipsError);
            AlwaysAssert(near(var(pos), expVar, 1e-8), AipsError);
            AlwaysAssert(near(sigma(pos), expSigma, 1e-11), AipsError);
            AlwaysAssert(near(rms(pos), expRMS, 1e-8), AipsError);
            AlwaysAssert(mymin(pos) == DComplex(0, 0), AipsError);
            AlwaysAssert(mymax(pos) == DComplex(399999, 399999), AipsError);

            // masked lattice, no range
            Vector<Bool> mask(size);
            Vector<Bool>::iterator miter = mask.begin();
            Vector<Bool>::iterator mend = mask.end();
            Bool mval = False;
            for (; miter!=mend; ++miter) {
                *miter = mval;
                mval = ! mval;
            }
            ArrayLattice<Bool> mlatt(mask);
            expSum = DComplex(62500000000, 62500000000);
            expNpts = DComplex(250000, 0);
            expSumSq = DComplex(0, 41666666666378080);
            expMean = expSum/expNpts;
            expNVar = DComplex(0, 10416666666500000);
            expVar = expNVar/(expNpts - 1);
            expSigma = sqrt(expVar);
            expRMS = sqrt(expSumSq/expNpts);
            subLatt.setPixelMask(mlatt, True);
            statsOld.setNewLattice(subLatt);
            statsOld.setInExCludeRange(Vector<Complex>(), Vector<Complex>());
            statsOld.getStatistic(sum, LatticeStatsBase::SUM);
            statsOld.getStatistic(npts, LatticeStatsBase::NPTS);
            statsOld.getStatistic(mean, LatticeStatsBase::MEAN);
            statsOld.getStatistic(sumsq, LatticeStatsBase::SUMSQ);
            statsOld.getStatistic(var, LatticeStatsBase::VARIANCE);
            statsOld.getStatistic(sigma, LatticeStatsBase::SIGMA);
            statsOld.getStatistic(rms, LatticeStatsBase::RMS);
            statsOld.getStatistic(mymax, LatticeStatsBase::MAX);
            statsOld.getStatistic(mymin, LatticeStatsBase::MIN);
            AlwaysAssert(sum(pos) == expSum, AipsError);
            AlwaysAssert(npts(pos) == expNpts, AipsError);
            AlwaysAssert(mean(pos) == expMean, AipsError);
            AlwaysAssert(near(sumsq(pos), expSumSq, 1e-8), AipsError);
            AlwaysAssert(near(var(pos), expVar, 1e-8), AipsError);
            AlwaysAssert(near(sigma(pos), expSigma, 1e-11), AipsError);
            AlwaysAssert(near(rms(pos), expRMS, 1e-8), AipsError);
            AlwaysAssert(mymin(pos) == DComplex(1, 1), AipsError);
            AlwaysAssert(mymax(pos) == DComplex(499999, 499999), AipsError);

            statsNew.setNewLattice(subLatt);
            statsNew.setInExCludeRange(Vector<Complex>(), Vector<Complex>());
            statsNew.getStatistic(sum, LatticeStatsBase::SUM);
            statsNew.getStatistic(npts, LatticeStatsBase::NPTS);
            statsNew.getStatistic(mean, LatticeStatsBase::MEAN);
            statsNew.getStatistic(sumsq, LatticeStatsBase::SUMSQ);
            statsNew.getStatistic(var, LatticeStatsBase::VARIANCE);
            statsNew.getStatistic(sigma, LatticeStatsBase::SIGMA);
            statsNew.getStatistic(rms, LatticeStatsBase::RMS);
            statsNew.getStatistic(mymax, LatticeStatsBase::MAX);
            statsNew.getStatistic(mymin, LatticeStatsBase::MIN);
            AlwaysAssert(sum(pos) == expSum, AipsError);
            AlwaysAssert(npts(pos) == expNpts, AipsError);
            AlwaysAssert(mean(pos) == expMean, AipsError);
            AlwaysAssert(near(sumsq(pos), expSumSq, 1e-8), AipsError);
            AlwaysAssert(near(var(pos), expVar, 1e-8), AipsError);
            AlwaysAssert(near(sigma(pos), expSigma, 1e-11), AipsError);
            AlwaysAssert(near(rms(pos), expRMS, 1e-8), AipsError);
            AlwaysAssert(mymin(pos) == DComplex(1, 1), AipsError);
            AlwaysAssert(mymax(pos) == DComplex(499999, 499999), AipsError);

            // mask with include range
            expSum = DComplex(75000000, 75000000);
            expNpts = DComplex(5000, 0);
            expSumSq = DComplex(0, 2333333330000);
            expMean = expSum/expNpts;
            expNVar = DComplex(0, 83333330000);
            expVar = expNVar/(expNpts - 1);
            expSigma = sqrt(expVar);
            expRMS = sqrt(expSumSq/expNpts);
            statsOld.setInExCludeRange(include, Vector<Complex>());
            statsOld.getStatistic(sum, LatticeStatsBase::SUM);
            statsOld.getStatistic(npts, LatticeStatsBase::NPTS);
            statsOld.getStatistic(mean, LatticeStatsBase::MEAN);
            statsOld.getStatistic(sumsq, LatticeStatsBase::SUMSQ);
            statsOld.getStatistic(var, LatticeStatsBase::VARIANCE);
            statsOld.getStatistic(sigma, LatticeStatsBase::SIGMA);
            statsOld.getStatistic(rms, LatticeStatsBase::RMS);
            statsOld.getStatistic(mymax, LatticeStatsBase::MAX);
            statsOld.getStatistic(mymin, LatticeStatsBase::MIN);
            AlwaysAssert(sum(pos) == expSum, AipsError);
            AlwaysAssert(npts(pos) == expNpts, AipsError);
            AlwaysAssert(mean(pos) == expMean, AipsError);
            AlwaysAssert(near(sumsq(pos), expSumSq, 2e-8), AipsError);
            AlwaysAssert(near(var(pos), expVar, 1e-8), AipsError);
            AlwaysAssert(near(sigma(pos), expSigma, 1e-11), AipsError);
            AlwaysAssert(near(rms(pos), expRMS, 1e-8), AipsError);
            AlwaysAssert(mymin(pos) == DComplex(10001, 10001), AipsError);
            AlwaysAssert(mymax(pos) == DComplex(19999, 19999), AipsError);

            statsNew.setInExCludeRange(include, Vector<Complex>());
            statsNew.getStatistic(sum, LatticeStatsBase::SUM);
            statsNew.getStatistic(npts, LatticeStatsBase::NPTS);
            statsNew.getStatistic(mean, LatticeStatsBase::MEAN);
            statsNew.getStatistic(sumsq, LatticeStatsBase::SUMSQ);
            statsNew.getStatistic(var, LatticeStatsBase::VARIANCE);
            statsNew.getStatistic(sigma, LatticeStatsBase::SIGMA);
            statsNew.getStatistic(rms, LatticeStatsBase::RMS);
            statsNew.getStatistic(mymax, LatticeStatsBase::MAX);
            statsNew.getStatistic(mymin, LatticeStatsBase::MIN);
            AlwaysAssert(sum(pos) == expSum, AipsError);
            AlwaysAssert(npts(pos) == expNpts, AipsError);
            AlwaysAssert(mean(pos) == expMean, AipsError);
            AlwaysAssert(near(sumsq(pos), expSumSq, 2e-8), AipsError);
            AlwaysAssert(near(var(pos), expVar, 1e-8), AipsError);
            AlwaysAssert(near(sigma(pos), expSigma, 1e-11), AipsError);
            AlwaysAssert(near(rms(pos), expRMS, 1e-8), AipsError);
            AlwaysAssert(mymin(pos) == DComplex(10001, 10001), AipsError);
            AlwaysAssert(mymax(pos) == DComplex(19999, 19999), AipsError);

            // mask with exclude range
            expSum = DComplex(40000000000, 40000000000);
            expNpts = DComplex(200000, 0);
            expSumSq = DComplex(0, 21333333333178080);
            expMean = expSum/expNpts;
            expNVar = DComplex(0, 5333333333200000);
            expVar = expNVar/(expNpts - 1);
            expSigma = sqrt(expVar);
            expRMS = sqrt(expSumSq/expNpts);
            statsOld.setInExCludeRange(Vector<Complex>(), exclude);
            statsOld.getStatistic(sum, LatticeStatsBase::SUM);
            statsOld.getStatistic(npts, LatticeStatsBase::NPTS);
            statsOld.getStatistic(mean, LatticeStatsBase::MEAN);
            statsOld.getStatistic(sumsq, LatticeStatsBase::SUMSQ);
            statsOld.getStatistic(var, LatticeStatsBase::VARIANCE);
            statsOld.getStatistic(sigma, LatticeStatsBase::SIGMA);
            statsOld.getStatistic(rms, LatticeStatsBase::RMS);
            statsOld.getStatistic(mymax, LatticeStatsBase::MAX);
            statsOld.getStatistic(mymin, LatticeStatsBase::MIN);
            AlwaysAssert(sum(pos) == expSum, AipsError);
            AlwaysAssert(npts(pos) == expNpts, AipsError);
            AlwaysAssert(mean(pos) == expMean, AipsError);
            AlwaysAssert(near(sumsq(pos), expSumSq, 2e-8), AipsError);
            AlwaysAssert(near(var(pos), expVar, 1e-8), AipsError);
            AlwaysAssert(near(sigma(pos), expSigma, 1e-11), AipsError);
            AlwaysAssert(near(rms(pos), expRMS, 1e-8), AipsError);
            AlwaysAssert(mymin(pos) == DComplex(1, 1), AipsError);
            AlwaysAssert(mymax(pos) == DComplex(399999, 399999), AipsError);

            statsNew.setInExCludeRange(Vector<Complex>(), exclude);
            statsNew.getStatistic(sum, LatticeStatsBase::SUM);
            statsNew.getStatistic(npts, LatticeStatsBase::NPTS);
            statsNew.getStatistic(mean, LatticeStatsBase::MEAN);
            statsNew.getStatistic(sumsq, LatticeStatsBase::SUMSQ);
            statsNew.getStatistic(var, LatticeStatsBase::VARIANCE);
            statsNew.getStatistic(sigma, LatticeStatsBase::SIGMA);
            statsNew.getStatistic(rms, LatticeStatsBase::RMS);
            statsNew.getStatistic(mymax, LatticeStatsBase::MAX);
            statsNew.getStatistic(mymin, LatticeStatsBase::MIN);
            AlwaysAssert(sum(pos) == expSum, AipsError);
            AlwaysAssert(npts(pos) == expNpts, AipsError);
            AlwaysAssert(mean(pos) == expMean, AipsError);
            AlwaysAssert(near(sumsq(pos), expSumSq, 2e-8), AipsError);
            AlwaysAssert(near(var(pos), expVar, 1e-8), AipsError);
            AlwaysAssert(near(sigma(pos), expSigma, 1e-11), AipsError);
            AlwaysAssert(near(rms(pos), expRMS, 1e-8), AipsError);
            AlwaysAssert(mymin(pos) == DComplex(1, 1), AipsError);
            AlwaysAssert(mymax(pos) == DComplex(399999, 399999), AipsError);
        }
        {
            // CAS-10938
            cout << "test CAS-10938" << endl;
            Array<Float> largeArray(IPosition(3, 4000, 4000, OMP::nMaxThreads() + 1));
            ArrayLattice<Float> myLatt(largeArray);
            SubLattice<Float> mySubLatt(myLatt);
            LatticeStatistics<Float> lattStats(mySubLatt);
            Vector<Int> axes(2, 0);
            axes[1] = 1;
            lattStats.setAxes(axes);
            // ensure the stats framework code branch will be used
            lattStats.configureClassical(1, 1, 0, 0);
            Array<Double> npts;
            // The fact that this call completes successfully is
            // sufficient to verify the bug fix
            AlwaysAssert(lattStats.getStatistic(npts, LatticeStatsBase::NPTS), AipsError);
        }
        {
            Vector<Float> mydata(2);
            mydata[0] = 0;
            mydata[1] = 1;
            ArrayLattice<Float> latt(mydata);
            SubLattice<Float> subLatt(latt);
            LatticeStatistics<Float> stats(subLatt);
            stats.configureFitToHalf(
                FitToHalfStatisticsData::CVALUE,
                FitToHalfStatisticsData::LE_CENTER
            );
            stats.setComputeQuantiles(True);
            Array<Double> v;
            stats.getStatistic(v, LatticeStatsBase::MEAN, False);
            AlwaysAssert(*v.begin() == 0, AipsError);
            // fix for issue found in as part of CAS-10948 implementation
            // successful completion of the call verifies the fix
            stats.getStatistic(v, LatticeStatsBase::Q3, False);
            AlwaysAssert(*v.begin() == 0, AipsError);
            stats.getStatistic(v, LatticeStatsBase::Q1, False);
            AlwaysAssert(*v.begin() == 0, AipsError);
        }
        {
            // tests using the various lattice stats algorithms
            IPosition shape(3, 100, 100, 100);
            Array<Float> adata(shape);
            indgen(adata);
            ArrayLattice<Float> latt(adata);
            SubLattice<Float> subLatt(latt);
            LogIO log;
            for (uInt i=0; i<3; ++i) {
                LatticeStatistics<Float> stats(subLatt, log);
                stats.setComputeQuantiles(True);
                switch(i) {
                case 0:
                    stats.forceUseOldTiledApplyMethod();
                    break;
                case 1:
                    stats.forceUseStatsFrameworkUsingArrays();
                    break;
                case 2:
                    stats.forceUseStatsFrameworkUsingDataProviders();
                    break;
                }
                Array<Double> stat;
                stats.getStatistic(stat, LatticeStatsBase::SUM);
                AlwaysAssert(*stat.begin() == 499999500000, AipsError);
                stats.getStatistic(stat, LatticeStatsBase::SUMSQ);
                AlwaysAssert(near(*stat.begin(), (Double)333332833333500000, 1e-9), AipsError);
                stats.getStatistic(stat, LatticeStatsBase::MEAN);
                AlwaysAssert(*stat.begin() == 499999.5, AipsError);
                stats.getStatistic(stat, LatticeStatsBase::RMS);
                AlwaysAssert(near(*stat.begin(), 577349.8361764728, 1e-9), AipsError);
                stats.getStatistic(stat, LatticeStatsBase::VARIANCE);
                AlwaysAssert(near(*stat.begin(), 83333416666.666672, 1e-9), AipsError);
                stats.getStatistic(stat, LatticeStatsBase::SIGMA);
                AlwaysAssert(near(*stat.begin(), 288675.2789323441, 1e-9), AipsError);
                stats.getStatistic(stat, LatticeStatsBase::MAX);
                AlwaysAssert(*stat.begin() == 999999, AipsError);
                stats.getStatistic(stat, LatticeStatsBase::MIN);
                AlwaysAssert(*stat.begin() == 0, AipsError);
                stats.getStatistic(stat, LatticeStatsBase::MEDIAN);
                AlwaysAssert(*stat.begin() == 499999.5, AipsError);
                stats.getStatistic(stat, LatticeStatsBase::Q1);
                AlwaysAssert(*stat.begin() == 249999, AipsError);
                stats.getStatistic(stat, LatticeStatsBase::Q3);
                AlwaysAssert(*stat.begin() == 749999, AipsError);
                stats.getStatistic(stat, LatticeStatsBase::QUARTILE);
                AlwaysAssert(*stat.begin() == 500000, AipsError);
                stats.getStatistic(stat, LatticeStatsBase::MEDABSDEVMED);
                AlwaysAssert(*stat.begin() == 250000, AipsError);
                IPosition minPos, maxPos;
                stats.getMinMaxPos(minPos, maxPos);
                AlwaysAssert(minPos == IPosition(3, 0, 0 ,0), AipsError);
                AlwaysAssert(maxPos == shape-1, AipsError);
                stats.setAxes(Vector<Int>(1, 1));
                IPosition loc(2, 50, 50);
                stats.getStatistic(stat, LatticeStatsBase::SUM);
                AlwaysAssert(stat(loc) == 50500000, AipsError);
                stats.getStatistic(stat, LatticeStatsBase::SUMSQ);
                AlwaysAssert(near(stat(loc), (Double)25503333250000, 1e-8), AipsError);
                stats.getStatistic(stat, LatticeStatsBase::MEAN);
                AlwaysAssert(stat(loc) == 505000.0, AipsError);
                stats.getStatistic(stat, LatticeStatsBase::RMS);
                AlwaysAssert(near(stat(loc), 505008.24993261247, 1e-8), AipsError);
                stats.getStatistic(stat, LatticeStatsBase::VARIANCE);
                AlwaysAssert(near(stat(loc), 8416666.666666666, 1e-9), AipsError);
                stats.getStatistic(stat, LatticeStatsBase::SIGMA);
                AlwaysAssert(near(stat(loc), 2901.149197588202, 1e-9), AipsError);
                stats.getStatistic(stat, LatticeStatsBase::MAX);
                AlwaysAssert(stat(loc) == 509950, AipsError);
                stats.getStatistic(stat, LatticeStatsBase::MIN);
                AlwaysAssert(stat(loc) == 500050, AipsError);
                stats.getStatistic(stat, LatticeStatsBase::MEDIAN);
                AlwaysAssert(stat(loc) == 505000.0, AipsError);
                stats.getStatistic(stat, LatticeStatsBase::Q1);
                AlwaysAssert(stat(loc) == 502450, AipsError);
                stats.getStatistic(stat, LatticeStatsBase::Q3);
                AlwaysAssert(stat(loc) == 507450, AipsError);
                stats.getStatistic(stat, LatticeStatsBase::QUARTILE);
                AlwaysAssert(stat(loc) == 5000, AipsError);
                stats.getStatistic(stat, LatticeStatsBase::MEDABSDEVMED);
                AlwaysAssert(stat(loc) == 2500, AipsError);
                stats.getMinMaxPos(minPos, maxPos);
                AlwaysAssert(minPos.empty(), AipsError);
                AlwaysAssert(maxPos.empty(), AipsError);
            }
        }
    }
    catch (const std::exception& x) {
        cerr << "aipserror: error " << x.what() << endl;
        return 1;
    }
    return 0;
}
 
void doitFloat (LogIO& os) 
{   
      
// Construct lattice

   IPosition shape(1);
   shape = 64;
   Array<Float> inArr(shape);
   indgen(inArr);
//
//
   Vector<Float> results(LatticeStatsBase::NSTATS);
   Vector<Bool> hasResult(LatticeStatsBase::NSTATS);
   hasResult = True;
//
   results(LatticeStatsBase::NPTS) = Float(shape(0));
   results(LatticeStatsBase::SUM) = sum(inArr);
   results(LatticeStatsBase::SUMSQ) = sum(square(inArr));
   Float med = median(inArr);
   results(LatticeStatsBase::MEDIAN) = med;
   results(LatticeStatsBase::MEDABSDEVMED) = median(abs(inArr-med));
   Float t1 = fractile(inArr, 0.25);
   Float t2 = fractile(inArr, 0.75);
   results(LatticeStatsBase::QUARTILE) = (t2-t1);
   results(LatticeStatsBase::Q1) = t1;
   results(LatticeStatsBase::Q3) = t2;
   results(LatticeStatsBase::MIN) = min(inArr);
   results(LatticeStatsBase::MAX) = max(inArr);
   results(LatticeStatsBase::MEAN) = mean(inArr);
   results(LatticeStatsBase::VARIANCE) = variance(inArr);
   results(LatticeStatsBase::SIGMA ) = stddev(inArr);
   results(LatticeStatsBase::RMS ) = rms(inArr);
//
   hasResult(LatticeStatsBase::FLUX) = False;

// Make 1D Lattice and test

   do1DFloat(results, hasResult, inArr, os);

// Make 2D lattice and test

   do2DFloat(results, hasResult, inArr, os);
}


void do1DFloat (const Vector<Float>& results,
                const Vector<Bool>& hasResult, 
                const Array<Float>& inArr, LogIO& os)
{
   const IPosition shape = inArr.shape();
   ArrayLattice<Float> inLat(inArr);
   SubLattice<Float> subLat(inLat);
   LatticeStatistics<Float> stats(subLat, os, False, False);

   test1DFloat (stats, results, hasResult, shape);

// Test copy constructor - feeble test
     
   {
      LatticeStatistics<Float> stats2(stats);
      test1DFloat (stats2, results, hasResult, shape);
   }

// Test assignment operator - feeble test
   
   {
      LatticeStatistics<Float> stats2(stats);
      stats = stats2;
      test1DFloat (stats, results, hasResult, shape);
   }

// Test setNewLattice - feeble test

   {
      AlwaysAssert(stats.setNewLattice(subLat), AipsError);
      test1DFloat (stats, results, hasResult, shape);
   }
}


void do2DFloat (const Vector<Float>& results,
                const Vector<Bool>& hasResult, 
                const Array<Float>& arr, LogIO& os)
{
   uInt nX = arr.shape()(0);
   uInt nY = 20;
   IPosition shape(2,nX,nY);

// Fill Lattice with replicated rows

   ArrayLattice<Float> lat(shape);
   Slicer slice(IPosition(2,0,0),shape,Slicer::endIsLength);
   LatticeUtilities::replicate (lat, slice, arr);
   SubLattice<Float> subLat(lat);

// Make LS object and set axes so that we work out stats
// over first axis as a function of nY replicated rows

   LatticeStatistics<Float> stats(subLat, os, False, False);
   Vector<Int> axes(1);
   axes = 0;
   AlwaysAssert(stats.setAxes(axes), AipsError);

// Test

   test2DFloat (stats, results, hasResult, shape);

// Test copy constructor - feeble test
     
   {
      LatticeStatistics<Float> stats2(stats);
      test2DFloat (stats2, results, hasResult, shape);
   }

// Test assignment operator - feeble test
   
   {
      LatticeStatistics<Float> stats2(stats);
      stats = stats2;
      test2DFloat (stats, results, hasResult, shape);
   }

// Test setNewLattice - feeble test

   {
      AlwaysAssert(stats.setNewLattice(subLat), AipsError);
      test2DFloat (stats, results, hasResult, shape);
   }
}

void test1DFloat (LatticeStatistics<Float>& stats, const Vector<Float>& results,
                  const Vector<Bool>& hasResult, const IPosition& shape)
{
   AlwaysAssert(stats.displayAxes().nelements()==0, AipsError);
//
   typedef NumericTraits<Float>::PrecisionType AccumType;
   Double tol = 1.0e-6;

   {
      IPosition pos(1,0);
      Vector<AccumType> data;
      AlwaysAssert(stats.getStats(data, pos, True), AipsError);
   }

   {
      const Int nStats = LatticeStatsBase::NSTATS;
      for (Int i=0; i<nStats; i++) {
        Array<AccumType> a;
        LatticeStatsBase::StatisticsTypes t = static_cast<LatticeStatsBase::StatisticsTypes>(i);
        IPosition pos(1,0);

        if (t == LatticeStatsBase::FLUX) {
           AlwaysAssert(!stats.getStatistic (a, t, True), AipsError);
        } else {
           AlwaysAssert(stats.getStatistic (a, t, True), AipsError);
        }
        if (hasResult(i)) {
           AlwaysAssert(a.shape() == IPosition(1,1),AipsError);
           AlwaysAssert(near(a(pos), results(i), tol), AipsError);
        }
        Array<Float> b;
        if (t==LatticeStatsBase::FLUX) {
           AlwaysAssert(!stats.getConvertedStatistic (b, t, True), AipsError);
        } else {
           AlwaysAssert(stats.getConvertedStatistic (b, t, True), AipsError);
        }
        if (hasResult(i)) {
           AlwaysAssert(b.shape()==IPosition(1,1),AipsError);
           AlwaysAssert(near(b(pos),results(i),tol), AipsError);
        }
      }
   }

   {
      IPosition minPos, maxPos;
      AlwaysAssert(stats.getMinMaxPos(minPos, maxPos), AipsError);
      AlwaysAssert(minPos.nelements()==1, AipsError);      
      AlwaysAssert(minPos(0)==0, AipsError);      
      AlwaysAssert(maxPos(0)=shape(0)-1, AipsError);
   }

   {
      Float dMin, dMax;
      AlwaysAssert(stats.getFullMinMax (dMin, dMax), AipsError);
      AlwaysAssert(near(results(LatticeStatsBase::MIN),dMin,tol), AipsError);
      AlwaysAssert(near(results(LatticeStatsBase::MAX),dMax,tol), AipsError);
   }
}


void test2DFloat (LatticeStatistics<Float>& stats, const Vector<Float>& results,
                  const Vector<Bool>& hasResult, const IPosition& shape)
{
   AlwaysAssert(shape.nelements()==2,AipsError);
   const Vector<Int> dA = stats.displayAxes();
   AlwaysAssert(dA.nelements()==1, AipsError);
   AlwaysAssert(dA(0)==1, AipsError);
   const uInt nY = shape(1);
//
   typedef NumericTraits<Float>::PrecisionType AccumType;
   Double tol = 1.0e-6;
//
   {
      IPosition pos(2,0,0);
      Vector<AccumType> data;
      AlwaysAssert(stats.getStats(data, pos, True), AipsError);
      AlwaysAssert(data.shape()==IPosition(1,LatticeStatsBase::NSTATS),AipsError);
   }

// Check stats correct for each row 

   {
      const Int nStats = LatticeStatsBase::NSTATS;
      for (Int i=0; i<nStats; i++) {
        Array<AccumType> a;
        LatticeStatsBase::StatisticsTypes t = static_cast<LatticeStatsBase::StatisticsTypes>(i);
        IPosition pos(1,0);
//
        if (t==LatticeStatsBase::FLUX) {
           AlwaysAssert(!stats.getStatistic (a, t, True), AipsError);
        } else {
           AlwaysAssert(stats.getStatistic (a, t, True), AipsError);
        }
        if (hasResult(i)) {
           AlwaysAssert(a.shape()==IPosition(1,nY),AipsError);
           for (uInt j=0; j<nY; j++) {
              pos(0) = j;
              AlwaysAssert(near(a(pos),results(i),tol), AipsError);
           }
        }
//
        Array<Float> b;
        if (t==LatticeStatsBase::FLUX) {
           AlwaysAssert(!stats.getConvertedStatistic (b, t, True), AipsError);
        } else {
           AlwaysAssert(stats.getConvertedStatistic (b, t, True), AipsError);
        }
        if (hasResult(i)) {
           AlwaysAssert(b.shape()==IPosition(1,nY),AipsError);
           for (uInt j=0; j<nY; j++) {
              pos(0) = j;
              AlwaysAssert(near(b(pos),results(i),tol), AipsError);
           }
        }
      }
   }
//  
   {
      Float dMin, dMax;
      AlwaysAssert(stats.getFullMinMax (dMin, dMax), AipsError);
      AlwaysAssert(near(results(LatticeStatsBase::MIN),dMin,tol), AipsError);
      AlwaysAssert(near(results(LatticeStatsBase::MAX),dMax,tol), AipsError);
   }
}

