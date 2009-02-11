librule(name="lbfgs",
    headers=["lbfgs.h", "lbfgs_impl.h", "optimization_utils.h"],
    deplibs=["fastlib:fastlib"])

binrule(name="test",
    sources=["test_lbfgs.cc"],
    deplibs=["fastlib:fastlib", ":lbfgs", "mlpack/allknn:allknn", 
    "contrib/nvasil/allkfn:allkfn", "contrib/nvasil/mvu:mvu"])

