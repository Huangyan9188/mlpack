#include <fastlib/fastlib.h>

#include <armadillo>
#include <fastlib/base/arma_compat.h>

void FindIndexWithPrefix(Dataset &dataset, char *prefix,
			 std::vector<index_t> &remove_indices, 
			 bool keep_going_after_first_match) {

  // Get the dataset information containing the feature types and
  // names.
  DatasetInfo &info = dataset.info();
  std::vector<DatasetFeature> &features = info.features();

  for(index_t i = 0; i < features.size(); i++) {

    // If a feature name with the desired prefix has been found, then
    // make sure it hasn't been selected before. If so, then add to
    // the remove indices.
    const std::string &feature_name = features[i].name();

    if(!strncmp(prefix, feature_name.c_str(), strlen(prefix) - 1)) {
      
      bool does_not_exist_yet = true;
      for(index_t j = 0; j < remove_indices.size(); j++) {
	if(remove_indices[j] == i) {
	  does_not_exist_yet = false;
	  break;
	}
      }
      if(does_not_exist_yet) {
	printf("Found: %s at position %"LI".\n", feature_name.c_str(), i);
	remove_indices.push_back(i);
	
	if(!keep_going_after_first_match) {
	  break;
	}
      }
    }    
  }  
}

int main(int argc, char *argv[]) {
  fx_init(argc, argv, NULL);

  // Read in the dataset from the file.
  Dataset initial_dataset;
  const char *dataset_name = fx_param_str_req(fx_root, "data");
  if(initial_dataset.InitFromFile(dataset_name) != SUCCESS_PASS) {
    FATAL("Could ont read the dataset %s", dataset_name);
  }

  // Now examine each feature name of the dataset, and construct the
  // indices.
  std::vector<index_t> remove_indices;
  char buffer[1000];
  do {
    printf("Input the prefix of the feature that you want to remove ");
    printf("(just press enter if you are done): ");
    fgets(buffer, 998, stdin);

    if(strlen(buffer) == 1) {
      break;
    }
    FindIndexWithPrefix(initial_dataset, buffer, remove_indices, false);
  } while(true);

  std::vector<index_t> prune_indices;
  do {
    printf("Input the prefix of the feature that you want to consider for pruning ");
    printf("(just press enter if you are done): ");
    fgets(buffer, 998, stdin);
    
    if(strlen(buffer) == 1) {
      break;
    }
    FindIndexWithPrefix(initial_dataset, buffer, prune_indices, true);
  } while(true);

  // Output the indices to the file based on the results above.
  FILE *predictor_file = fopen("predictor_indices.csv", "w+");
  FILE *prune_file = fopen("prune_indices.csv", "w+");

  for(index_t i = 0; i < initial_dataset.matrix().n_rows; i++) {
    bool to_be_removed = false;
    for(index_t j = 0; j < remove_indices.size(); j++) {
      if(remove_indices[j] == i) {
	to_be_removed = true;
	break;
      }
    }
    if(!to_be_removed) {
      fprintf(predictor_file, "%"LI"\n", i);
    }
  }
  for(index_t i = 0; i < prune_indices.size(); i++) {
    fprintf(prune_file, "%"LI"\n", prune_indices[i]);
  }
  fclose(predictor_file);
  fclose(prune_file);

  fx_timer_start(fx_root, "qr_time");
  Matrix q, r, tmp;
  arma_compat::armaToMatrix(initial_dataset.matrix(), tmp);
  la::QRInit(tmp, &q, &r);
  fx_timer_stop(fx_root, "qr_time");
  printf("%"LI" %"LI" %"LI" %"LI"\n", q.n_rows(), q.n_cols(), r.n_rows(), r.n_cols());

  fx_done(fx_root);
  return 0;
}
