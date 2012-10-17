#include <shogun/labels/MulticlassLabels.h>
#include <shogun/features/DenseFeatures.h>
#include <shogun/kernel/GaussianKernel.h>
#include <shogun/multiclass/MulticlassLibSVM.h>
#include <shogun/lib/SGSparseMatrix.h>
#include <shogun/io/AsciiFile.h>
#include <shogun/features/SparseFeatures.h>
#include <shogun/base/init.h>

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
using namespace shogun;

void print_message(FILE* target, const char* str)
{
	fprintf(target, "%s", str);
}

int main(int argc, char** argv)
{
	//init_shogun(&print_message);
	shogun::sg_io = new shogun::SGIO();
	sg_io->set_loglevel(MSG_DEBUG);
	init_shogun_with_defaults();
	index_t num_vec=100;
	index_t num_feat=100;
	index_t num_class=4;

	// create some data
	SGSparseMatrix<float64_t> matrix(num_feat, num_vec);
	//SGVector<float64_t>::range_fill_vector(matrix.matrix, num_feat*num_vec);
	char train_fname[32] = "train.input";
	FILE *train_file = fopen(train_fname,"r");
	CAsciiFile *Cfile=new CAsciiFile(train_file,train_fname);
	Cfile->get_sparse_matrix(matrix.sparse_matrix,num_feat,num_vec);
	matrix.num_vectors = num_vec;
	matrix.num_features = num_feat;


	std::cout<< "matrix(m*n):" <<matrix.num_vectors << " " <<  matrix.num_features << std::endl;
	for (int i=0 ; i< matrix.num_vectors ;i++)
	{
		for(int j=0; j<matrix.sparse_matrix[i].num_feat_entries;j++)
		{
			std::cout <<  matrix.sparse_matrix[i].features[j].feat_index << ":" << matrix.sparse_matrix[i].features[j].entry << " ";
		}
		std::cout << std::endl;
	}


	// create vectors
	// shogun will now own the matrix created
	CSparseFeatures<float64_t>* features=new CSparseFeatures<float64_t>(matrix);

	// create three labels
	char fname_label[32] = "labels.input";
	CAsciiFile *Cfile_label=new CAsciiFile((const char *)fname_label,'r',(const char *)fname_label);
	CMulticlassLabels* labels=new CMulticlassLabels(Cfile_label);
	//CMulticlassLabels* labels=new CMulticlassLabels(num_vec);
	//for (index_t i=0; i<num_vec; ++i)
	//	labels->set_label(i, i%num_class);
	//labels->save(Cfile_label);

	// create gaussian kernel with cache 10MB, width 0.5
	CGaussianKernel* kernel = new CGaussianKernel(10, 0.5);
	kernel->init(features, features);

	// create libsvm with C=10 and train
	CMulticlassLibSVM* svm = new CMulticlassLibSVM(10, kernel, labels);
	svm->train();

	// classify on training examples
	CMulticlassLabels* output=CMulticlassLabels::obtain_from_generic(svm->apply());
	SGVector<float64_t>::display_vector(output->get_labels().vector, output->get_num_labels(),
			"batch output");

	/* assert that batch apply and apply(index_t) give same result */
	for (index_t i=0; i<output->get_num_labels(); ++i)
	{
		float64_t label=svm->apply_one(i);
		SG_SPRINT("single output[%d]=%f\n", i, label);
	//	ASSERT(output->get_label(i)==label);
	}
	//SG_UNREF(output);

	// free up memory
	SG_UNREF(svm);

	exit_shogun();
	return 0;
}

