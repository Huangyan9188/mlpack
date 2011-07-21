#include "cosine_tree.h"

// L2 norm of a specific column in a matrix
double columnNormL2(const Matrix& A, index_t i_col) {
  Vector col;
  A.MakeColumnVector(i_col, &col);
  return la::LengthEuclidean(col);
}

// Zero tolerance
#define eps (1e-16)
bool isZero(double d) {
  return (d<eps) && (d>-eps);
}

// Init a root cosine node from a matrix
CosineNode::CosineNode(const Matrix& A) {
  this->A_.Alias(A);
  origIndices_.reserve(A_.n_cols());
   norms_.reserve(A_.n_cols());

   int newval = A_.n_cols();


   for (index_t i_col = 0; i_col < A_.n_cols(); i_col++) {
        origIndices_.push_back(0);      
        norms_.push_back(0);
    }

 
   for (index_t i_col = 0; i_col < A_.n_cols(); i_col++) {
        origIndices_[i_col] = i_col;
        norms_[i_col] = columnNormL2(A_, i_col);
  }

  parent_ = NULL;
  left_ = NULL;
  right_ = NULL;
  isLeft_ = false;

  CalStats();
}

// Init a child cosine node from its parent and a set of the parent's columns
CosineNode::CosineNode(CosineNode& parent, 
		       const std::vector<int>& indices, bool isLeft) {
  A_.Alias(parent.A_);
  origIndices_.reserve(indices.size());
  norms_.reserve(n_cols());
   
for(index_t i_col = 0; i_col < indices.size(); i_col++) {    
      norms_.push_back(0);
      origIndices_.push_back(0);
   }

  for (index_t i_col = 0; i_col < origIndices_.size(); i_col++) {
    origIndices_[i_col] = parent.origIndices_[indices[i_col]];
    norms_[i_col] = parent.norms_[indices[i_col]];
  }
  parent_ = &parent;
  left_ = NULL;
  right_ = NULL;
  isLeft_ = isLeft;
  
  if (isLeft) parent.left_ = this;
  else parent.right_ = this;
  CalStats();
}

void CosineNode::CalStats() {
  // Calculate cummlulative sum square of L2 norms

 cum_norms_.reserve(A_.n_cols());

  for (index_t i_col = 0; i_col < A_.n_cols(); i_col++) {
     cum_norms_.push_back(0);
  }  

  for (index_t i_col = 0; i_col < A_.n_cols(); i_col++) {
  cum_norms_[i_col] = ((i_col > 0) ? cum_norms_[i_col-1]:0)
      + math::Sqr(norms_[i_col]);
  }
  // Calculate mean vector
  mean_.Init(A_.n_rows()); mean_.SetZero();
  for (index_t i_col = 0; i_col < n_cols(); i_col++) {
    Vector col;
    GetColumn(i_col, &col);
    la::AddTo(col, &mean_);
  }
  la::Scale(1.0/(double)origIndices_.size(), &mean_);
}

void CosineNode::ChooseCenter(Vector* center) {

  double r = (double)rand()/RAND_MAX * cum_norms_[n_cols()-1];
  index_t i_col;
  for (i_col = 0; i_col < n_cols(); i_col++)
    if (cum_norms_[i_col] >= r) break;
  GetColumn(i_col, center);
}

void CosineNode::CalCosines(const Vector& center, 
			    std::vector<double>* cosines) {

  for(index_t i_col=0; i_col < n_cols(); i_col++) {
    (*cosines).push_back(0);
  }

  double centerL2 = la::LengthEuclidean(center);
 
 for (index_t i_col = 0; i_col < n_cols(); i_col++)
  { 
    // if col is a zero vector then push it to the left node
    if (isZero(norms_[i_col]))
      (*cosines)[i_col] = 2;
    else {
      Vector col;
      GetColumn(i_col, &col);
      double numerator = la::Dot(center, col);
      double denominator = centerL2 * norms_[i_col];
      (*cosines)[i_col] = numerator/denominator;
    }
  }
}

void CosineNode::CreateIndices(std::vector<int>* indices) {
  indices->reserve(n_cols());
 
   for (index_t i_col = 0; i_col < n_cols(); i_col++)
   {
      (*indices).push_back(0);
   }

   for (index_t i_col = 0; i_col < n_cols(); i_col++)
    (*indices)[i_col] = i_col;
}

// Quicksort partitioning procedure
index_t qpartition(std::vector<double>& key, std::vector<int>& data,
		index_t left, index_t right) {
  index_t j = left;
  double x = key[left];
  
 for (index_t i = left+1; i <= right; i++)
  {    
 
   if (key[i] >= x) {
      j++;
      double tmp_d = key[i]; key[i] = key[j]; key[j] = tmp_d;
      index_t tmp_i = data[i]; data[i] = data[j]; data[j] = tmp_i;
    }
   }

  double tmp_d = key[left]; key[left] = key[j]; key[j] = tmp_d;
  index_t tmp_i = data[left]; data[left] = data[j]; data[j] = tmp_i;
  return j;
}


// Quicksort on the cosine values
void qsort(std::vector<double>& key, std::vector<int>& data,
	   index_t left, index_t right) {

 if (left >= right) return;
  index_t middle = qpartition(key, data, left, right);
  qsort(key, data, left, middle-1);
  qsort(key, data, middle+1, right);
}

void Sort(std::vector<double>& key, std::vector<int>& data) {
  qsort(key, data, 0, key.size()-1);
}

// Calculate the split point where the cosines values are closer
// to the minimum cosine value than the maximum cosine value
index_t calSplitPoint(const std::vector<double>& key) {
  double leftKey = key[0];
  double rightKey = key[key.size()-1];
  index_t i = 0;
  while (leftKey - key[i] <= key[i] - rightKey && i < key.size()) i++;
  //printf("i = %d\n", i);
  return i;
}

// Init a subcopy of an array list
void InitSubCopy(const std::vector<int>& src, index_t pos, index_t size,
		 std::vector<int>* dst) {

   dst->reserve(size);
   for (index_t i = 0; i < size; i++)
    (*dst).push_back(0);

   for (index_t i = 0; i < size; i++)
    (*dst)[i] = src[pos+i];
}

// Split the indices at the split point
void splitIndices(std::vector<int>& indices, int leftSize,
		  std::vector<int>* leftIdx, std::vector<int>* rightIdx) {
  InitSubCopy(indices, 0, leftSize, leftIdx);
  InitSubCopy(indices, leftSize, indices.size()-leftSize, rightIdx);
}

// Split a cosine tree node by choose a random center, and sort
// the cosine values decreasingly, then choose a split point
// by calling calSplitPoint()
// This procedure won't split a node if either child has the same
// set of columns as the parent
void CosineNode::Split() {

  if (n_cols() < 2) return;
  Vector center;
  std::vector<double> cosines;
  std::vector<int> indices, leftIdx, rightIdx;

  ChooseCenter(&center);

  CalCosines(center, &cosines);

  CreateIndices(&indices);

  Sort(cosines, indices);

  index_t leftSize = calSplitPoint(cosines);

  if (leftSize == n_cols() || leftSize == 0) return;
  
  splitIndices(indices, leftSize, &leftIdx, &rightIdx);
  
  left_ = new CosineNode(*this, leftIdx, true);
  right_ = new CosineNode(*this, rightIdx, false);
}

