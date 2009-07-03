librule(
	name = "svm_projection",
	headers = ["svm_projection.h"],
        sources = ["project_simplex.cc", "min_quad_on_simplex.cc"],
      	deplibs = ["fastlib:fastlib"],
)

librule(
	name = "svm",
	headers = ["svm.h"],
        sources = ["seqminopt.cc"],
      	deplibs = ["fastlib:fastlib"],
)

binrule(
	# The name of the executable.
	name = "svm_projection_test",
	
	# The .c or .cc file containing main and any others you need.
	sources = ["svm_projection_test.cc"],
	
	# This line can be omitted if there are no headers.
	# headers = ["util.h","quic_svd_base.h","cosine_tree.h"],
	
	# The leading colon means to check this build.py for allnn.
	deplibs = [":svm_projection"]
)

