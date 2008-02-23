#ifndef KNN_REGRESSION_H
#define KNN_REGRESSION_H
#define LEAF_SIZE 1
#define CALCULATE_FOR_REFERENCE_POINTS_ 0
#define CALCULATE_FOR_QUERY_POINTS_ 1

#include <fastlib/fastlib.h>
#include "allknn.h"
#include "pseudo_inverse.h"


//This is a templatized class

template<typename TKernel> class KNNRegression{

 private:

  //An array to hold the index of neighbours and distances for all the query points

  ArrayList<index_t> nn_neighbours_;
  ArrayList<double> nn_distances_;


  //The confidence intervals calculation requires us to calculate the
  //regression estimates at all the reference points too.

  ArrayList<index_t> nn_neighbours_reference_points_;
                   
  ArrayList<double> nn_distances_reference_points_;



  //The bandwidth to be used
  double bandwidth_;


  //The kind of kernel to be used
  TKernel kernel_;


  //The regression estimates for the query points
  Vector regression_estimates_;

  //The regression estimates for the reference points. This is done by
  //local fitting methods but now the query points are the same as the
  //reference points

  Vector regression_estimates_reference_points_;

  //The reference dataset

  Matrix rset_;
  
  //The query dataset

  Matrix qset_;
  
  //The reference values
  
  Vector rset_weights_;
  

  //The number of neighbours u want

  index_t k_;

  //Matrices that are required for BTWB and BTWY calculations for
  //Local linear calculations. They will store
  //(B^TWB)^-1 and (B^TWY) ^-1 after the call to the PseudoInverse
  //calculation

  ArrayList <Matrix> b_twb_;
  ArrayList <Matrix> b_twy_;
  ArrayList <Matrix> b_tw2b_;

  //Matrices that are requierd for BTWB and BTWY calculations for
  //local linear calculations on the reference set. They will store
  //(B^TWB)^-1 and (B^TWY) ^-1 after the call to the PseudoInverse
  //calculation

  ArrayList <Matrix> b_twb_reference_points_;
  ArrayList <Matrix> b_twy_reference_points_;


  //They store the upper and lower bounds of the confidence intervals
  //for each query point
  Vector confidence_interval_;


  //This stores the squares of the mean residuals
  double sigma_hat_;

  //This stores the knn distance of all the reference points
  Vector knn_distances_;





  /** This function calculates the influence of the observation at a
   *  reference point on its own regression estimate
   */

      
  void CalculateSigmaHatKNNLocalLinear_(){

    // [1,r] (B^TWB)^-1 (B^T W e_i)

    //lets first calculate B^TW . We first form the B matrix by usign
    //the k-nearest neighbours of the reference point under
    //consideration


   
    Matrix point;
    point.Init(1,rset_.n_rows()+1);
    point.set(0,0,1);

    Matrix influence_matrix;
    influence_matrix.Init(1,1);

    double total_influence=0;
    double total_sqd_influence=0;

    Matrix temp;
    temp.Init(1,rset_.n_rows()+1);


    Matrix point_transpose;
    point_transpose.Init(rset_.n_rows()+1,1);

    double sqd_residue=0;
    
    for(index_t i=0;i<rset_.n_cols();i++){
      
      //Now the influence of the point i is 
      //influence=[1,r] (B^TWB)^-1 [1,r]^T


      for(index_t j=1;j<rset_.n_rows()+1;j++){
	point.set(0,j,rset_.get(j-1,i));
      }

    
      //This multiplies [1,r] (B^TWB)^-1 and stores in temp. Note
      //b_twb_reference_point stores the inverse of BTWB matrix of a
      //point

      la::MulOverwrite(point,b_twb_reference_points_[i],&temp);

     
      //Now multiply temp with [1,r]^T
      //First lets transpose point and store it in point_transpose
 
      la::TransposeOverwrite(point,&point_transpose);
      la::MulOverwrite(temp,point_transpose,&influence_matrix);
      total_influence+=influence_matrix.get(0,0);
     
      total_sqd_influence+=influence_matrix.get(0,0)*influence_matrix.get(0,0);

      double residue=regression_estimates_reference_points_[i]-rset_weights_[i];
      sqd_residue+=residue*residue;
    }

    double df1=total_influence;
    double df2=total_sqd_influence;

    printf("df1 is %f\n",df1);
    printf("df2 is %f\n",df2);
    //Now calculate the squared residues

    sigma_hat_=sqrt((1/(rset_.n_cols()-2*df1+df2))*sqd_residue);
    printf("sigma hat is %f\n",sigma_hat_);
  }


  void CalculateSigmaHatKNNNWR_(){

   
    double sum_of_squared_residual_errors=0;
    double df1=0.0;
    double df2=0.0;

    for(index_t r=0;r<rset_.n_cols();r++){
      
      //For each reference point. Compute the kernel sum of its knn
      
      double sum=0;
      for(index_t l=0;l<k_;l++){
	
	sum+=kernel_.EvalUnnormOnSq(nn_distances_reference_points_[r*k_+l]);
      }

      //So we now have the sum of the kernel weights of all the
      //knn. We shall proceed to calculate the degrees of freedom usig
      //these values

      df1+=1/sum; //(1/sum) is the influence of the reference point on its own estimate
      df2+=1/(sum*sum);  

      //We also need to run over all the reference points and compute the
      //squared residual error.

      printf("regression estimate for the reference point is %f\n",regression_estimates_reference_points_[r]);
      sum_of_squared_residual_errors+=
	(rset_weights_[r]-regression_estimates_reference_points_[r])*
	(rset_weights_[r]-regression_estimates_reference_points_[r]);
      
    }

    printf("degress of freedom1 are %f\n",df1);
    printf("degrees of freedom2 are %f\n",df2);

    sigma_hat_=sqrt(sum_of_squared_residual_errors/(rset_.n_cols()-2*df1+df2));
  }
  
  double CalculateTheSqdLengthOfHatVectorKNNNWR_(index_t q){

    double sum=0;

    for(index_t l=0;l<k_;l++){
      
      sum+=kernel_.EvalUnnormOnSq(nn_distances_[q*k_+l]);
    }

    double sqdlength=0;
    for(index_t l=0;l<k_;l++){
      
      double contribution_due_to_other_point=kernel_.EvalUnnormOnSq(nn_distances_[q*k_+l]);
      double ratio=contribution_due_to_other_point/sum;
      sqdlength+=ratio*ratio;
    }

    printf("The squared length of hat vector for the query point q=%d is %f\n",q,sqdlength);
    return sqdlength;
  }

  double CalculateTheSqdLengthOfHatVectorKNNLocalLinear_(index_t q){

    // ([1,q] (B^TWB)^-1) (B^T W^2 B) ((B^TWB)^-1[1,q]T)
    
    //Note we already have B^TWB^-1

    

    Matrix point;
    point.Init(1,qset_.n_rows()+1);
    point.set(0,0,1);

    for(index_t j=1;j<qset_.n_rows()+1;j++){

      point.set(0,j,qset_.get(j-1,q));
    }

  

    Matrix left;
    
    la::MulInit(point,b_twb_[q],&left);
   

    Matrix right;
    la::TransposeInit(left,&right);

    //Note:Middle matrix has already been calculated during Compute
    //Matrix calculations. all that remains left to be done is multiply these three matrices

    Matrix temp;
    la::MulInit(b_tw2b_[q],right,&temp);

    Matrix sqd_length; //this is the squared length of the l(x) vector
    la::MulInit(left,temp,&sqd_length);
    return sqd_length.get(0,0);

  }

  void ComputeConfidenceIntervalKNNLocalLinear_(){

 

    //perform local linear regression on the reference set of points
    KNNLocalLinearRegression_(flag);


    //having done this we need the influence of the reference element
    //on its own regression estimate

    CalculateSigmaHatKNNLocalLinear_();

    //The next task is to calculate the length of ||l(x)|| for each
    //and every query point

    for(index_t q=0;q<qset_.n_cols();q++){
      
      double sqdlength= CalculateTheSqdLengthOfHatVectorKNNLocalLinear_(q);
      
      //Once we have the length of the hat vector we should be able to
      //calculate the upper and lower bounds of the C.I for the query
      //point

      printf("regression estimate is %f\n",regression_estimates_reference_points_[q]);
      double lower_bound=regression_estimates_reference_points_[q]-1.96*sigma_hat_*(1+sqdlength);
      confidence_interval_[q*2]=lower_bound;

      double upper_bound=regression_estimates_reference_points_[q]+1.96*sigma_hat_*(1+sqdlength);
      confidence_interval_[q*2+1]=upper_bound;

      printf("Uopper bound is %f\n",upper_bound);
      printf("lower bound id %f\n",lower_bound);
    }
   
  }


  
  void ComputeConfidenceIntervalKNNNWR_(){
      
    //The C.I requires us to calculate the regression fits at all the
    //reference points. hence we first check if the query set and the
    //reference set are the same. If they are then we dont need to do
    //anything special. if they are not then we first calculate the
    //fits at all the refernece points and then go on tc calculate the
    //confidence intervals
      
 
    //So now we need to do a local fitting at each and every reference
    //point

    
  
    KNNNWRegression_(flag);
    //printf("Did KNNNWR regression...\n");
    
    
    
    //We need to do two things to compute the C.I. of an point. First
    //we need to calculate the sigma_hat and then we need to calculate
    //the length of l(x) vector. Lets do them in 2 functions.Note that
    //the sigma_hat is not query dependent and is same for all the query points
    
    CalculateSigmaHatKNNNWR_();

    //printf("sigma hat is %f\n",sigma_hat_);

    //The next important thing to do is to calculate the length of
    //||l(x)|| vector. Lets call this vector as the hat vector
    
    for(index_t q=0;q<qset_.n_cols();q++){
      
      double sqdlength= CalculateTheSqdLengthOfHatVectorKNNNWR_(q);
      
      //Once we have the length of the hat vector we should be able to
      //calculate the upper and lower bounds of the C.I for the query
      //point
      double lower_bound=regression_estimates_[q]+1.96*sigma_hat_*(1+sqdlength);
      confidence_interval_[q*2]=lower_bound;

      double upper_bound=regression_estimates_[q]+1.96*sigma_hat_*(1+sqdlength);
      confidence_interval_[q*2+1]=upper_bound;

      printf("Uopper bound is %f\n",upper_bound);
      printf("lower bound id %f\n",lower_bound);
    }
  }



  void KNNNWRegression_(index_t flag){   
    
    //For each query point
    
    index_t number_of_points;
    if(flag==CALCULATE_FOR_QUERY_POINTS_){
      
      number_of_points = qset_.n_cols();
    }
    
    //we need to calculate the regression estimates at the reference
    //points too

    else{

      number_of_points = rset_.n_cols();
    }
    
   

    for(index_t q=0; q<number_of_points; q++){
      printf("q is %d\n",q);
      
      float numerator=0;
      float denominator=0;

      //For each neighbour
      
      if(flag==CALCULATE_FOR_QUERY_POINTS_){ 
       
	for(index_t l=0;l<k_;l++){
	  
	  numerator+=
	    rset_weights_[nn_neighbours_[q*k_+l]]*
	    kernel_.EvalUnnormOnSq (nn_distances_[q*k_+l]); 
	  
	  denominator+=kernel_.EvalUnnormOnSq(nn_distances_[q*k_+l]);
	}
	regression_estimates_[q]=numerator/denominator;
      }
      
      else{
      
	
	for(index_t l=0;l<k_;l++){

	    

	  numerator+=
	    rset_weights_[nn_neighbours_reference_points_[q*k_+l]]*
	    kernel_.EvalUnnormOnSq (nn_distances_reference_points_[q*k_+l]); 
	  
	  denominator+=
	    kernel_.EvalUnnormOnSq(nn_distances_reference_points_[q*k_+l]);
	}
	regression_estimates_reference_points_[q]=numerator/denominator;

      }
    }
  }
  


  //Note this function is called to calculate the matrices for the
  //query points

  void ComputeMatrices(){
    
    for (index_t q = 0; q < qset_.n_cols (); q++){	//for each query point
      const double *q_col = qset_.GetColumnPtr (q);
      
      //for each reference point which are the k-nearest neighbours of
      //the query point
      
      for (index_t r = 0; r < k_; r++){ 
	
	//Get reference point
	const double *r_col = rset_.GetColumnPtr(nn_neighbours_[q*k_+r]);
	
	// pairwise distance and kernel value
	double dsqd =
	  la::DistanceSqEuclidean (qset_.n_rows (), q_col, r_col);
	
	double ker_value = kernel_.EvalUnnormOnSq (dsqd);
	
	for(index_t col = 0; col < rset_.n_rows () + 1; col++){	//along each direction
	  
	  for(index_t row=0; row< rset_.n_rows () + 1; row++){
	    
	    
	    //Lets gradually fill up all the elements of the matrices 
	    //b_twy_ and b_twb_
	    
	    //Fill in b_twy_ first
	    
	    if(col==0){
	      
	      //For this column 
	      if (row != 0){
		
		//Fill B^TWY naive
		
		double val=b_twy_[q].get(row,col)+ 
		  ker_value * rset_weights_[nn_neighbours_[q*k_+r]] * 
		  rset_.get (row- 1,nn_neighbours_[q*k_+r]);
		
		b_twy_[q].set(row,col,val);
		
		//Now fill B^TWB naive
		
		double val1=b_twb_[q].get(row,col) + 
		  ker_value * rset_.get(row-1,nn_neighbours_[q*k_+r]);
		b_twb_[q].set(row,col,val1);

		//Also fill in B^T W^2 B matrix

		double val2=b_tw2b_[q].get(row,col) + 
		  ker_value * ker_value*rset_.get(row-1,nn_neighbours_[q*k_+r]);
		b_tw2b_[q].set(row,col,val2);
		
	      }
	      
	      else{
		
		//Fill B^TWY naive 
		
		double val= b_twy_[q].get(row,col) + 
		  ker_value * rset_weights_[nn_neighbours_[q*k_+r]];
		
		b_twy_[q].set(row,col,val);
		
		//Now fill B^TWB
		
		double val1=b_twb_[q].get(row,col) 
		  + ker_value ;
		
		b_twb_[q].set(row,col,val1);

		//Also fill B^T W^2 B matrix
		double val2=b_tw2b_[q].get(row,col) 
		  + ker_value *ker_value;
		
		b_tw2b_[q].set(row,col,val2);
		
		
	      }
	    }//end of col 0...............
	    
	    //Column!=0
	    
	    else{
	      if(row!=0){
		//Only B^TWB naive estimates get filled up
		double val1=b_twb_[q].get(row,col) 
		  + ker_value* rset_.get(row-1,nn_neighbours_[q*k_+r])* 
		  rset_.get(col-1,nn_neighbours_[q*k_+r]);
		
		b_twb_[q].set(row,col,val1);

		double val2=b_tw2b_[q].get(row,col) 
		  + ker_value*ker_value* rset_.get(row-1,nn_neighbours_[q*k_+r])* 
		  rset_.get(col-1,nn_neighbours_[q*k_+r]);
		
		b_tw2b_[q].set(row,col,val2);

	      }
	      
	      else{
		double val1=b_twb_[q].get(row,col) 
		  + ker_value* rset_.get(col-1,nn_neighbours_[q*k_+r]);
		b_twb_[q].set(row,col,val1);

		double val2=b_tw2b_[q].get(row,col) 
		  + ker_value* ker_value*rset_.get(col-1,nn_neighbours_[q*k_+r]);
		b_tw2b_[q].set(row,col,val2);
	      }
	    }
	  }
	}
      }
    }
  }
  
  //Note for this function we are calculating the matrices for the
  //reference points. So the q_matrix is same as the r_matrix

  void ComputeMatricesForReferencePoints(){

    printf("compute matrices for reference points...\n");    

    for (index_t q = 0; q < rset_.n_cols (); q++){	

      const double *q_col = rset_.GetColumnPtr (q);
      
      //for each reference point which are the k-nearest neighbours of
      //the query point 
      
      for (index_t r = 0; r < k_; r++){ 
	
	//Get reference point
	const double *r_col = rset_.GetColumnPtr(nn_neighbours_reference_points_[q*k_+r]);
	
	// pairwise distance and kernel value
	double dsqd =
	  la::DistanceSqEuclidean (rset_.n_rows (), q_col, r_col);
	
	double ker_value = kernel_.EvalUnnormOnSq (dsqd);
	
	for(index_t col = 0; col < rset_.n_rows () + 1; col++){	//along each direction
	  
	  for(index_t row=0; row< rset_.n_rows () + 1; row++){
	    
	    
	    //Lets gradually fill up all the elements of the matrices 
	    //b_twy_ and b_twb_
	    
	    //Fill in b_twy_ first
	    
	    if(col==0){
	      
	      //For this column 
	      if (row != 0){
		
		//Fill B^TWY naive
		
		double val=b_twy_reference_points_[q].get(row,col)+ 
		  ker_value * rset_weights_[nn_neighbours_reference_points_[q*k_+r]] * 
		  rset_.get (row- 1,nn_neighbours_reference_points_[q*k_+r]);
		
		b_twy_reference_points_[q].set(row,col,val);
		
		//Now fill B^TWB naive
		
		double val1=b_twb_reference_points_[q].get(row,col) + 
		  ker_value * rset_.get(row-1,nn_neighbours_reference_points_[q*k_+r]);
		b_twb_reference_points_[q].set(row,col,val1);
		
	      }
	      
	      else{
		
		//Fill B^TWY naive 
		
		double val= b_twy_reference_points_[q].get(row,col) + 
		  ker_value * 
		  rset_weights_[nn_neighbours_reference_points_[q*k_+r]];
		
		//printf("val is %f\n",val);
		b_twy_reference_points_[q].set(row,col,val);
		
		//Now fill B^TWB
		
		double val1=b_twb_reference_points_[q].get(row,col) 
		  + ker_value ;
		
		//printf("BTWB val is %f\n",val1);
		b_twb_reference_points_[q].set(row,col,val1);
	      }
	    }//end of col 0...............
	    
	    //Column!=0
	    
	    else{

	      if(row!=0){
		//Only B^TWB naive estimates get filled up
		double val1=b_twb_reference_points_[q].get(row,col) 
		  + ker_value* rset_.get(row-1,nn_neighbours_reference_points_[q*k_+r])* 
		  rset_.get(col-1,nn_neighbours_reference_points_[q*k_+r]);
		
		b_twb_reference_points_[q].set(row,col,val1);

	      }
	      
	      else{
		double val1=b_twb_reference_points_[q].get(row,col) 
		  + ker_value* rset_.get(col-1,nn_neighbours_reference_points_[q*k_+r]);
		b_twb_reference_points_[q].set(row,col,val1);
	      }
	    }
	  }
	}
      }
    }
  }
  
  void KNNLocalLinearRegression_(index_t flag){
    
    //This will involve calculation of 2 matrices. One is BTWB and
    //another is BTWY.First lets compute BTWB


    if(flag==CALCULATE_FOR_QUERY_POINTS_){

      //In this case since we are doing regression estimates on the query points.
      //the query set is  just the original reference set

     
      ComputeMatrices();
    }
    else{

      //here we are doing regression on the reference points 
     
      ComputeMatricesForReferencePoints();
    }

   

    index_t number_of_points;

    if(flag==CALCULATE_FOR_QUERY_POINTS_){
      
      number_of_points=qset_.n_cols();

      ArrayList<Matrix> temp;
      temp.Init(number_of_points);

      //This returns the inverse of the matrix BTWB
      
      for(index_t q=0;q<number_of_points;q++){
	
	//With this b_twb_[q] holds the pseudo inverse of the matrix B^TWB 


	PseudoInverse::FindPseudoInverse(b_twb_[q]);  





	la::MulInit (b_twb_[q],b_twy_[q], &temp[q]);
	Matrix q_matrix;
	
	//Initialize q matrix and set it up. The q_matrix will have a
	//query point 
	
	q_matrix.Init(1,qset_.n_rows()+1);
	q_matrix.set(0,0,1);
	
	for(index_t col=0;col<qset_.n_rows();col++){
	  
	  q_matrix.set(0,col+1,rset_.get(col,q));
	}
	  
	//Now lets multiply the resulting q_matrix with temp to get the
	//regression estimate
	
	Matrix temp2;
	la::MulInit(q_matrix,temp[q],&temp2);  
	
	regression_estimates_[q]=temp2.get(0,0);
	
      }

      printf("regression estimates for the query points are..\n");
      regression_estimates_.PrintDebug();
    }

    else{
	
      number_of_points=rset_.n_cols();
      ArrayList<Matrix> temp;
      temp.Init(number_of_points);
      
      //This returns the inverse of the matrix BTWB
      
      for(index_t q=0;q<number_of_points;q++){




	//With this b_twb_[q] holds the pseudo inverse of the matrix B^TWB 
	PseudoInverse::FindPseudoInverse(b_twb_reference_points_[q]);  


	la::MulInit (b_twb_reference_points_[q],b_twy_reference_points_[q], &temp[q]);
	Matrix q_matrix;
	
	//Initialize q matrix and set it up. The q_matrix will have a
	//query point 
	
	q_matrix.Init(1,rset_.n_rows()+1);
	q_matrix.set(0,0,1);
	
	for(index_t col=0;col<rset_.n_rows();col++){
	  
	  q_matrix.set(0,col+1,rset_.get(col,q));
	}
	  
	//Now lets multiply the resulting q_matrix with temp to get the
	//regression estimate
	
	Matrix temp2;
	la::MulInit(q_matrix,temp[q],&temp2);  
	
	regression_estimates_reference_points_[q]=temp2.get(0,0);
      }

      printf("Priniting regression estimates for reference points ..\n");
      regression_estimates_reference_points_.PrintDebug();
     
    }
  }
    
  void GetTheKNNNeighbours_(index_t flag){
    
    
    //First lets get the knn of all the query points We shall
    //accomplish this by calling the functions related to the object
    
    AllkNN *all_knn;
    all_knn=new AllkNN();
  

    if(flag==CALCULATE_FOR_QUERY_POINTS_){

     
      all_knn->Init(rset_,rset_,LEAF_SIZE,k_);
     
      //Now lets call the compute function.
      all_knn->ComputeNeighbors(&nn_neighbours_,&nn_distances_);

      
    }

    else{
    
      all_knn->Init(rset_,rset_,LEAF_SIZE,k_);
      all_knn->ComputeNeighbors
	(&nn_neighbours_reference_points_,&nn_distances_reference_points_);
    }
  }



 public:
 
  
  void Compute(const char *method){
    

    //lets declare an object of type AllKNNDualTree to find out the
    //k-nearest neighbours by dual tree recurusion
    

    //We first find the knn of all the query points
    index_t flag=CALCULATE_FOR_QUERY_POINTS_;
    
    GetTheKNNNeighbours_(flag);


    //We also need to find out the knn of all the reference
    //points. This will act as a bandwidth


    index_t flag=CALCULATE_FOR_REFERENCE_POINTS_;
    
    GetTheKNNNeighbours_(flag);
    
    
    if(!strcmp(method,"nwr")){
      
      //Do KNN based Nadaraya Watson regression
      KNNNWRegression_(flag);
      ComputeConfidenceIntervalKNNNWR_();
    }
    
    else{
      
      //Do KNN based Local linear regression
      KNNLocalLinearRegression_(flag);
      ComputeConfidenceIntervalKNNLocalLinear_();
    }
    
  }

  void Init(index_t k, Matrix q_matrix, Matrix r_matrix, 
	    Vector rset_weights, double bandwidth){
    
    k_=k;
    bandwidth_=bandwidth;
   

    //Alias the query and reference datassets
    qset_.Copy(q_matrix);
    rset_.Copy(r_matrix);
  

    //Alias the rset_weights. These are the regression values of the
    //reference points and are know to us

    rset_weights_.Copy(rset_weights);

    //Initialize the kerneL
    kernel_.Init(bandwidth);

  

    //Also lets initialize regression estimates
    regression_estimates_.Init(qset_.n_cols());


    //Lets initialize the regression estimates of the reference points
    //too

    regression_estimates_reference_points_.Init(rset_.n_cols());

    //Lets initialize the matrices for local linear estimates

    b_twb_.Init(qset_.n_cols());
    b_tw2b_.Init(qset_.n_cols());
    b_twb_reference_points_.Init(rset_.n_cols());

    //Initialize each matrix now

    for(index_t q=0;q<qset_.n_cols();q++){

      b_twb_[q].Init(rset_.n_rows()+1,rset_.n_rows()+1);
      b_twb_[q].SetZero();

      b_tw2b_[q].Init(rset_.n_rows()+1,rset_.n_rows()+1);
      b_tw2b_[q].SetZero();

    }

    //similarily lets initialize BTWY too

    b_twy_.Init(qset_.n_cols());
    b_twy_reference_points_.Init(rset_.n_cols());

    for(index_t q=0;q<qset_.n_cols();q++){
      
      b_twy_[q].Init(rset_.n_rows()+1,1);
      b_twy_[q].SetZero();
    }

    for(index_t r=0;r<rset_.n_cols();r++){

      b_twy_reference_points_[r].Init(rset_.n_rows()+1,1);
      b_twy_reference_points_[r].SetZero();

      b_twb_reference_points_[r].Init(rset_.n_rows()+1,rset_.n_rows()+1);
      b_twb_reference_points_[r].SetZero();
    }

    //Initialize the confidence intervals also. Their size is twice
    //the size of the query set so as to store both the upper and
    //lower bounds

    confidence_interval_.Init(2*qset_.n_cols());
   
    //Set them to all 0's
    confidence_interval_.SetZero();

    //Store the knn distances of all the reference points

    knn_distances_.Init(rset_.n_cols());
    
  }
};

#endif
