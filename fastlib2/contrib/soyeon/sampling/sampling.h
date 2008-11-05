#include "fastlib/fastlib.h"


class Sampling {
	public:
	 void Init(fx_module *module);
	 void ExpandSubset(double percent_added_sample);

	private:
		//overlap with objective2.h
		//ArrayList<Matrix> first_stage_x_; 


		int num_people_;		//first_stage_x_.size()
		ArrayList<index_t> shuffled_array_; //length=num_people_

		//1 if initial_sampling is done, then need to expand_subset
		//0 if initial_sampling is not done yet
		int ind_initial_sampling_;	
		int num_initial_sampling_; //one of the argument

		ArrayList<index_t> sample_selector_; //
		
		void Shuffle();
};






