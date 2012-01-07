#ifndef RESPONSES_H_
#define RESPONSES_H_


class Responses
{
private:
public:
	// mozda na cuda-i da se izvrsava??? mada je glupo ako nisu .raw
	/**
	* @brief Weighting function with "flat" distribution (as in icip06)
	*
	* @param w [out] weights (array size of M)
	* @param M number of camera output levels
	*/
	void static exposure_weights_icip06( float* w, int M, int Mmin, int Mmax );

	/**
	* @brief Weighting function with triangle distribution (as in debevec)
	*
	* @param w [out] weights (array size of M)
	* @param M number of camera output levels
	*/
	void weights_triangle( float* w, int M/*, int Mmin, int Mmax */);

	/**
	* @brief Weighting function with gaussian distribution
	*
	* @param w [out] weights (array size of M)
	* @param M number of camera output levels
	* @param Mmin minimum registered camera output level
	* @param Mmax maximum registered camera output level
	* @param sigma sigma value for gaussian
	*/
	void static weightsGauss( float* w, int M, int Mmin, int Mmax, float sigma );

	/**
	* @brief Create gamma response function
	*
	* @param I [out] camera response function (array size of M)
	* @param M number of camera output levels
	*/
	void static responseGamma( float* I, int M );

	/**
	* @brief Create linear response function
	*
	* @param I [out] camera response function (array size of M)
	* @param M number of camera output levels
	*/
	void static responseLinear( float* I, int M );

	/**
	* @brief Create logarithmic response function
	*
	* @param I [out] camera response function (array size of M)
	* @param M number of camera output levels
	*/
	void static responseLog10( float* I, int M );
};




#endif /* RESPONSES_H_ */
