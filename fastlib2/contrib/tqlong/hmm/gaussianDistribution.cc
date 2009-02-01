#include "gaussianHMM.h"
#include "support.h"

using namespace supportHMM;

GaussianDistribution::GaussianDistribution(
		const Vector& mean, const Matrix& cov) {
	InitMeanCov(mean.length());
	setMeanCov(mean, cov);
}

GaussianDistribution::GaussianDistribution(index_t dim) {
	printf("TEST1\n");
	InitMeanCov(dim);
	Vector m; m.Init(dim);
	Matrix cov; cov.Init(dim, dim);
	
	printf("TEST1\n");
	
	for (index_t i = 0; i < dim; i++)
		m[i] = math::Random(-1, 1);
		
	printf("TEST1\n");
	
	cov.SetZero();
	for (index_t i = 0; i < dim; i++)
		cov.ref(i, i) = math::Random(1, 4);
	
	printf("TEST1\n");
	setMeanCov(m, cov);
	printf("TEST1\n");
}

GaussianDistribution::GaussianDistribution(const GaussianDistribution& gd) {
	mean.Copy(gd.mean);
	covariance.Copy(gd.covariance);
	invCov.Copy(gd.invCov);
	sqrCov.Copy(gd.sqrCov);
	accMean.Copy(gd.accMean);
	accCov.Copy(gd.accCov);
	gConst = gd.gConst;
	accDenom = gd.accDenom;
}

double GaussianDistribution::logP(const Vector& x) {
	return gConst - 0.5*multxAy(x, invCov, x);
}

void GaussianDistribution::createFromCols(
		const Matrix& src, index_t col, GaussianDistribution* tmp) {
	index_t dim = src.n_rows();
	Vector mean;
	Matrix covariance;
	src.MakeColumnVector(col, &mean);
	src.MakeColumnSlice(col+1, dim, &covariance);
	ot::Print(mean);
	ot::Print(covariance);
	tmp->setMeanCov(mean, covariance);
}

void GaussianDistribution::Generate(Vector* x) {
	RandomNormal(mean, sqrCov, x);
}

void GaussianDistribution::StartAccumulate() {
	accMean.SetZero();
	accCov.SetZero();
	accDenom = 0.0;
}

void GaussianDistribution::EndAccumulate() {
	la::ScaleInit(1.0/accDenom, accMean, &mean);
	Matrix meanMat;
	meanMat.AliasColVector(mean);
	la::ScaleOverwrite(1.0/accDenom, accCov, &covariance);
	la::MulExpert(-1.0, false, meanMat, true, meanMat, 1.0, &covariance);
}

void GaussianDistribution::Accumulate(const Vector& x, double weight) {
	la::AddExpert(weight, x, &accMean);
	Matrix xMat;
	xMat.AliasColVector(x);
	la::MulExpert(weight, false, xMat, true, xMat, 1.0, &accCov);
	accDenom += weight;
}

void GaussianDistribution::Save(FILE* f) {
	printVector(f, mean);
	printMatrix(f, covariance);
}

void GaussianDistribution::InitMeanCov(index_t dim) {
	mean.Init(dim);
	covariance.Init(dim, dim);
	
	invCov.Init(dim, dim);
	sqrCov.Init(dim, dim);
	
	accMean.Init(dim);
	accCov.Init(dim, dim);
}

void GaussianDistribution::setMeanCov(const Vector& m, const Matrix& cov) {
	this->mean.CopyValues(m);
	this->covariance.CopyValues(cov);
	printf("TEST2\n");
	ot::Print(cov);
	ot::Print(invCov);
	Matrix invTmp;
	la::InverseInit(cov, &invTmp);
	invCov.CopyValues(invTmp);
	printf("TEST2\n");
	Matrix tmp;
	la::CholeskyInit(cov, &tmp);
	printf("TEST2\n");
	sqrCov.CopyValues(tmp);
	printf("TEST2\n");
	gConst = -0.5*n_dim()*log(2*math::PI)-0.5*la::DeterminantLog(cov, NULL);
	printf("TEST2\n");
}

