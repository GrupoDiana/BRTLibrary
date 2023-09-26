#ifndef _SDN_UTILS_HPP_
#define _SDN_UTILS_HPP_

#include <Eigen/Core>
#include <Eigen/Dense>
#include <unsupported/Eigen/Polynomials>
#include <unsupported/Eigen/FFT>

using namespace Eigen;

class SDNUtils
{
public:


	/**
	* @brief Implementation of the MATLAB polystab function
	* @param a Polynomial to stabilize
	* @return stabilized polynomial
	*/
	static std::vector<double> polystab(std::vector<double>& a)
	{

		if (a.size() <= 1)
			return a;

		std::vector<double> aMirror(a.size());
		std::reverse_copy(a.begin(), a.end(), aMirror.begin());

		PolynomialSolver<double, Dynamic> solver;
		VectorXd aV = Map<VectorXd>(aMirror.data(), a.size());
		solver.compute(aV);
		VectorXcd v = solver.roots();

		int non0index = -1;

		for (int i = 0; i < v.size(); i++)
		{
			if (v[i] != std::complex<double>(0, 0))
			{
				double vs = 0.5 * (copysign(abs(v[i]) - 1 == 0 ? 0 : 1, abs(v[i]) - 1) + 1);
				v[i] = (1 - vs) * v[i] + vs / std::conj(v[i]);
				if (non0index < 0 && v[i] != std::complex<double>(0, 0))
				{
					non0index = i;
				}
			}
		}
		VectorXcd polCoeffs;
		roots_to_monicPolynomial(v, polCoeffs);

		VectorXd b = (a[non0index] * polCoeffs).real();

		std::vector<double> out(b.size());

		std::reverse_copy(b.data(), b.data() + b.size(), out.begin());

		return out;

	}

	/**
	* @brief Implementation of the MATLAB invfreqz function, does not work for filter orders higher than 7th
	* @param h Frequency response, specified as a complex vector
	* @param w Angular frequencies at which h is computed, specified as a double array
	* @param numOrder Desired order of the numerator polynomial
	* @param denOrder Desired order of the denominator polynomial
	* @param wSize size of the w array
	* @param weights Weighting factors, specified as a double array
	* @param iter Number of iterations in the search algorithm, default to 0
	* @param tol Tolerance, specified as a scalar. invfreqz defines convergence as occurring when the norm of the (modified) gradient vector is less than tol
	* @return Transfer function coefficients in format [b, a]
	*/
	static std::vector<std::vector<double>> invfreqz(std::complex<double>* h, double* w, int numOrder, int denOrder, int wSize,
		double* weights, int iter = 0, double tol = 0.01)
	{

		int nm = std::max(numOrder, denOrder);
		numOrder++;

		MatrixXd OM_m = VectorXd::LinSpaced(nm + 1, 0, nm) * Map<MatrixXd>(w, 1, wSize);

		std::complex<double> c(0, -1);

		MatrixXcd OM = OM_m * c;
		OM = OM.array().exp();


		MatrixXcd Dva_a = OM.block(1, 0, denOrder, OM.cols()).transpose();
		MatrixXcd h_t = Map<MatrixXcd>(h, 1, wSize).transpose();
		MatrixXcd Dva_b = h_t * MatrixXd::Ones(1, denOrder);

		MatrixXcd Dva = Dva_a.array() * Dva_b.array();
		MatrixXcd Dvb = -(OM.block(0, 0, numOrder, OM.cols()).transpose());

		MatrixXcd D(Dva.rows(), Dva.cols() + Dvb.cols());
		D << Dva, Dvb;

		MatrixXd wf = (Map<MatrixXd>(weights, 1, wSize).transpose()).cwiseSqrt();
		MatrixXd D_b = wf * MatrixXd::Ones(1, numOrder + denOrder);

		D = D.array() * D_b.array();

		MatrixXd R = (D.adjoint() * D).real();
		MatrixXd Vd = (D.adjoint() * (-h_t.array() * wf.array()).matrix()).real();

		MatrixXd th = R.partialPivLu().solve(Vd).eval();
		th = th.transpose();

		std::vector<double> a;
		std::vector<double> b;

		a.push_back(1);
		std::copy(th.data(), &th.data()[denOrder], back_inserter(a));
		std::copy(&th.data()[denOrder], &th.data()[denOrder + numOrder], back_inserter(b));

		if (iter == 0)
		{
			std::vector<std::vector<double>> out;
			out.push_back(b);
			out.push_back(a);

			return out;
		}

		a = polystab(a);

		VectorXd bV = Map<VectorXd>(b.data(), b.size());
		VectorXd aV = Map<VectorXd>(a.data(), a.size());

		MatrixXcd GC_b = bV.transpose() * OM.block(0, 0, numOrder, OM.cols());
		MatrixXcd GC_a = aV.transpose() * OM.block(0, 0, denOrder + 1, OM.cols());

		MatrixXcd GC = (GC_b.array() / GC_a.array()).transpose();
		MatrixXcd e = (GC - h_t).array() * wf.array();
		MatrixXcd Vcap = e.adjoint() * e;

		MatrixXd t(a.size() - 1 + b.size(), 1);
		t << Map<VectorXd>(&a.data()[1], a.size() - 1), bV;

		double gndir = 2 * tol + 1;
		int l = 0;
		int st = 0;
		MatrixXd gndirMat;

		while (gndir > tol && l < iter && st != 1)
		{
			l++;

			MatrixXcd D31_a = OM.block(1, 0, denOrder, OM.cols()).transpose();
			MatrixXcd D31_b = -GC.array() / (aV.transpose() * OM.block(0, 0, denOrder + 1, OM.cols())).transpose().array();
			MatrixXd D31_c = MatrixXd::Ones(1, denOrder);
			MatrixXcd D31 = D31_a.array() * (D31_b * D31_c).array();

			MatrixXcd D32_a = OM.block(0, 0, numOrder, OM.cols()).transpose();
			MatrixXcd D32_b = (aV.transpose() * OM.block(0, 0, denOrder + 1, OM.cols())).transpose();
			MatrixXd D32_c = MatrixXd::Ones(1, numOrder);
			MatrixXcd D32 = D32_a.array() / (D32_b * D32_c).array();

			MatrixXcd D3(D31.rows(), D31.cols() + D32.cols());
			D3 << D31, D32;
			MatrixXd D3_b = wf * MatrixXd::Ones(1, numOrder + denOrder);
			D3 = D3.array() * D3_b.array();

			e = (GC - h_t).array() * wf.array();
			R = (D3.adjoint() * D3).real();
			Vd = (D3.adjoint() * e).real();

			gndirMat = R.partialPivLu().solve(Vd).eval();
			int ll = 0;
			double k = 1.0;
			MatrixXcd V1 = Vcap.array() + 1;
			MatrixXd t1;

			while (V1(0, 0).real() > Vcap(0, 0).real() && ll < 20)
			{

				t1 = t - k * gndirMat;
				if (ll == 19)
					t1 = t;

				std::vector<double> t1_v(t1.transpose().data(), &t1.transpose().data()[t1.rows()]);
				std::copy(t1_v.begin(), t1_v.begin() + denOrder, a.begin() + 1);
				a = polystab(a);

				std::copy(a.begin() + 1, a.end(), t1_v.begin());
				std::copy(t1_v.begin() + denOrder, t1_v.end(), b.begin());

				bV = Map<VectorXd>(b.data(), b.size());
				aV = Map<VectorXd>(a.data(), a.size());

				GC_b = bV.transpose() * OM.block(0, 0, numOrder, OM.cols());
				GC_a = aV.transpose() * OM.block(0, 0, denOrder + 1, OM.cols());

				GC = (GC_b.array() / GC_a.array()).transpose();

				V1 = (GC - h_t).array() * wf.array();
				V1 = V1.adjoint() * V1;
				t1 = Map<VectorXd>(t1_v.data(), t1_v.size());

				k /= 2;
				ll++;
				if (ll == 20)
					st = 1;
				if (ll == 10)
				{
					gndirMat = Vd / R.norm() * R.rows();
					k = 1;
				}

			}

			t = t1;
			Vcap(0, 0) = V1(0, 0);

			gndir = gndirMat.norm();
		}

		std::vector<std::vector<double>> out;
		out.push_back(b);
		out.push_back(a);

		return out;

	}

	/**
	* @brief Linear interpolation of vector v with points defined in vector x to find values of interpPoints
	* @param x Vector of sample points
	* @param v Vector of the sample points values
	* @param interpPoints Vector of the new sample points to find the value of
	* @param out Vector to save the interpolated values in
	*/
	static void util_interp1(VectorXd& x, VectorXd& v, VectorXd& interpPoints, VectorXd& out)
	{
		if (interpPoints.size() == out.size())
		{
			int xIndex = 1;
			for (int i = 0; i < interpPoints.size(); i++)
			{
				while (x(xIndex) < interpPoints(i))
				{
					if (xIndex == x.size() - 1)
					{
						break;
					}
					xIndex++;
				}

				double interpCoefficient = (interpPoints(i) - x(xIndex - 1)) / (x(xIndex) - x(xIndex - 1));
				out(i) = v(xIndex - 1) + ((v(xIndex) - v(xIndex - 1)) * interpCoefficient);
			}
		}
	}


	/**
	* @brief Estimates the 3rd order filter transfer function coefficients given the octave bands absorption values
	* @param samplerate Sample rate of the filter
	* @param f125-f16000 absorption coefficients for each octave band from 125Hz to 16kHz, values must be in [0, 1]
	* @return Transfer function coefficients in format [b, a]
	*/
	static std::vector<std::vector<double>> getWallFilterCoeffs(double sampleRate, double f125, double f250, double f500, double f1000,
		double f2000, double f4000, double f8000, double f16000)
	{

		int N = 3;
		double	Fs = sampleRate;
		double sizeFFT = 1024;

		//
		//extrapolate and resample amplitude response
		//

		//starting data
		double amplitude[SDNParameters::NUM_FREQ] = { f125, f250, f500, f1000, f2000, f4000, f8000, f16000 };
		double freq[SDNParameters::NUM_FREQ] = { 125, 250, 500, 1000, 2000, 4000, 8000, 16000 };

		//convert absorption coefficients to reflectance data in Db
		for (int i = 0; i < SDNParameters::NUM_FREQ; i++)
		{
			amplitude[i] = amplitude[i] == 1 ? SDNParameters::MINUS_INFINITY_DB :
				std::max(SDNParameters::MINUS_INFINITY_DB, (std::log10(sqrt(1 - amplitude[i]))) * 20.0);
		}

		//extend data points to 0 and Nyquist freq
		double ampExtended[SDNParameters::NUM_FREQ + 2], freqExtended[SDNParameters::NUM_FREQ + 2];
		ampExtended[0] = amplitude[0];
		freqExtended[0] = 0;
		ampExtended[SDNParameters::NUM_FREQ + 1] = amplitude[SDNParameters::NUM_FREQ - 1];
		freqExtended[SDNParameters::NUM_FREQ + 1] = Fs / 2.0;
		std::copy(amplitude, &amplitude[SDNParameters::NUM_FREQ], &ampExtended[1]);
		std::copy(freq, &freq[SDNParameters::NUM_FREQ], &freqExtended[1]);

		//find interpolated reflectance values over the space [0, fs/2]
		VectorXd interpPoints = VectorXd::LinSpaced((sizeFFT / 2) + 1, 0, sizeFFT / 2);
		interpPoints *= (Fs / sizeFFT);
		int nSamples = interpPoints.size();

		VectorXd ampEV = Map<VectorXd>(ampExtended, SDNParameters::NUM_FREQ + 2);
		VectorXd freqEV = Map<VectorXd>(freqExtended, SDNParameters::NUM_FREQ + 2);

		VectorXd hInterp(nSamples);

		util_interp1(freqEV, ampEV, interpPoints, hInterp);


		//
		// Convert to minimum phase spectrum by folding cepstrum
		//

		//install negative prequencies on the spectrum
		VectorXcd logSpectrum(nSamples + nSamples - 2);
		logSpectrum << hInterp, hInterp.segment(1, nSamples - 2).reverse();

		//find the real cepstrum 
		FFT<double> fft;
		VectorXcd cepstrum((int)sizeFFT);
		fft.inv(cepstrum, logSpectrum);

		//fold the cepstrum to reflect the non minimum phase zeros inside the unit circle
		VectorXcd foldedCep((int)sizeFFT);
		foldedCep << cepstrum(0),
			cepstrum.segment(1, nSamples - 2) + cepstrum.segment(nSamples, nSamples - 2).reverse(),
			cepstrum(nSamples - 1),
			ArrayXcd::Zero((int)sizeFFT - nSamples);

		//find the minimum phase spectrum
		VectorXcd minPhLogSpectrum((int)sizeFFT);
		fft.fwd(minPhLogSpectrum, foldedCep);
		
		//select only the positive frequency portion
		VectorXcd hVec = minPhLogSpectrum.segment(0, nSamples);
		std::vector<std::complex<double>> h(hVec.data(), &hVec.data()[nSamples]);

		for (std::complex<double>& val : h)
		{
			val = pow(10.0, (val / 20.0));
		}

		//
		// Estimate filter coefficients
		// 
		
		//angular frequencies
		VectorXd w = (interpPoints / Fs) * _2PI;

		//ERB scale weights
		VectorXd wWeights = 1.0 / (24.7 * (4.37 * (interpPoints * 0.001).array() + 1));

		return invfreqz(h.data(), w.data(), N, N, w.size(), wWeights.data(), 10);

	}


	
};

//IIR filter implemented in direct form II, accepts any filter order
class IIRFilter
{
public:
	IIRFilter() {}
	~IIRFilter() {}
	
	/**
	* @brief Initialize the IIR filter variables
	* @param samplerate Samplerate of the filter
	* @param a Pointer to the denominator coefficients to be used by the filter
	* @param b Pointer to the numerator coefficients to be used by the filter
	*/
	void init(double samplerate, std::vector<double>* a, std::vector<double>* b)
	{
		sampleRate = samplerate;

		memory = std::vector<double>(std::max(a->size(), b->size()) - 1, 0.0f);

		this->a = a;
		this->b = b;
	}

	/**
	* @brief Pass a sample to the filter
	* @param sample Reference to the sample to be processed by the filter, this is going to get overwritten 
		with the process result
	*/
	void process(float& sample)
	{
		double inAcc, out;

		inAcc = sample * a->at(0);

		for (int i = 1; i < a->size(); i++)
		{
			inAcc -= a->at(i) * memory[i - 1];
		}

		out = inAcc * b->at(0);

		for (int i = 1; i < b->size(); i++)
		{
			out += b->at(i) * memory[i - 1];
		}
		sample = out;

		for (int i = memory.size() - 1; i >= 1; i--)
		{
			memory[i] = memory[i - 1];
		}

		memory[0] = inAcc;
	}


	/**
	* @brief Clear the filter memory by setting all stored samples to 0
	*/
	void clearMemory()
	{
		std::fill(memory.begin(), memory.end(), 0.0f);
	}

private:

	std::vector<double> memory;
	std::vector<double>* a, * b;

	double sampleRate;

};

#endif