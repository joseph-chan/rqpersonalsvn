/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 2008-2009 Soeren Sonnenburg
 * Copyright (C) 2008-2009 Fraunhofer Institute FIRST and Max Planck Society
 */
#include <shogun/kernel/GaussianKernel.h>
#include <shogun/features/SimpleFeatures.h>
#include <shogun/features/SparseFeatures.h>
#include <shogun/classifier/svm/LibSVM.h>
#include <shogun/io/AsciiFile.h>
#include <shogun/mathematics/Math.h>
#include <shogun/lib/common.h>
#include <shogun/base/init.h>

#include <stdlib.h>
#include <stdio.h>

using namespace shogun;

#define NUM 100
#define DIMS 2
#define DIST 0.5

float64_t* lab;
float64_t* feat;
float64_t* feat2;



void gen_rand_data()
{
	lab=SG_MALLOC(float64_t, NUM);
	feat=SG_MALLOC(float64_t, NUM*DIMS);
	feat2=SG_MALLOC(float64_t, NUM*DIMS);

	for (int32_t i=0; i<NUM; i++)
	{
		if (i<NUM/2)
		{
			lab[i]=-1.0;

			for (int32_t j=0; j<DIMS; j++)
			{
				feat[i*DIMS+j]=CMath::random(0,80)+DIST;
				feat2[i*DIMS+j]=CMath::random(0,80)+DIST;
			}
		}
		else
		{
			lab[i]=1.0;

			for (int32_t j=0; j<DIMS; j++)
			{
				feat[i*DIMS+j]=CMath::random(0,80)-DIST;
				feat2[i*DIMS+j]=CMath::random(0,80)-DIST;
			}
		}
	}
	CMath::display_vector(lab,NUM);
	//CMath::display_matrix(feat,DIMS, NUM);
	//CMath::display_matrix(feat2,DIMS, NUM);
}

int main()
{

	const int32_t feature_cache=0;
	const int32_t kernel_cache=0;
	const float64_t rbf_width=100;
	const float64_t svm_C=50;
	const float64_t svm_eps=0.001;

	init_shogun();

	gen_rand_data();


	// create train features
	//CFile(char* fname, char rw, const char* name
	//char fname[32]="test.input";
	//char rw = 'r';
	//CAsciiFile *test_file=new CAsciiFile(fname,rw,fname);
	
	/* create feature data matrix */
	index_t num_vectors=100;
	index_t num_features=1000;
	/* create some sparse data */
	SGSparseMatrix<float64_t> data=SGSparseMatrix<float64_t>(num_vectors,num_features);
	SGSparseMatrix<float64_t> data2=SGSparseMatrix<float64_t>(num_vectors,num_features);

	for (index_t i=0; i<num_vectors; ++i)
	{
		/* put elements only at even indices */
		data.sparse_matrix[i]=SGSparseVector<float64_t>(num_features, i);
		data2.sparse_matrix[i]=SGSparseVector<float64_t>(num_features, i);

		/* fill */
		for (index_t j=0; j<num_features; ++j)
		{
			if(i<NUM/2)
			{	
				if (j< num_features /2)
				{
					data.sparse_matrix[i].features[j].entry=CMath::random(0.0,1.0) + DIST;
					data.sparse_matrix[i].features[j].feat_index= j;

					data2.sparse_matrix[i].features[j].entry=CMath::random(0.0,1.0) + DIST;
					data2.sparse_matrix[i].features[j].feat_index= j;
				}
				else
				{
					data.sparse_matrix[i].features[j].entry=CMath::random(0.0,1.0) + DIST;
					data.sparse_matrix[i].features[j].feat_index= j+1;

					data2.sparse_matrix[i].features[j].entry=CMath::random(0.0,1.0) + DIST;
					data2.sparse_matrix[i].features[j].feat_index= j+1;
				}
			}
			else
			{
				if (j< num_features /2)
				{
					data.sparse_matrix[i].features[j].entry=CMath::random(0.0,1.0) + DIST;
					data.sparse_matrix[i].features[j].feat_index= j+1 ;

					data2.sparse_matrix[i].features[j].entry=CMath::random(0.0,1.0) + DIST;
					data2.sparse_matrix[i].features[j].feat_index= j+1 ;
				}
				else
				{
					data.sparse_matrix[i].features[j].entry=CMath::random(0.0,1.0) + DIST;
					data.sparse_matrix[i].features[j].feat_index= j ;

					data2.sparse_matrix[i].features[j].entry=CMath::random(0.0,1.0) + DIST;
					data2.sparse_matrix[i].features[j].feat_index= j ;
				}
			}

		}
	}

	// create train labels
	CLabels* labels=new CLabels(SGVector<float64_t>(lab, NUM));
	SG_REF(labels);

	CSparseFeatures<float64_t>* features = new CSparseFeatures<float64_t>(data);
	SG_REF(features);
	CSparseFeatures<float64_t>* features2 = new CSparseFeatures<float64_t>(data2);
	SG_REF(features2);

	// create gaussian kernel
	CGaussianKernel* kernel = new CGaussianKernel(kernel_cache, rbf_width);
	SG_REF(kernel);
	kernel->init(features, features);

	// create svm via libsvm and train
	CLibSVM* svm = new CLibSVM(svm_C, kernel, labels);
	SG_REF(svm);
	svm->set_epsilon(svm_eps);
	svm->train();

	printf("num_sv:%d b:%f\n", svm->get_num_support_vectors(), svm->get_bias());

	// classify + display output
	CLabels* out_labels=svm->apply(features2);

	int correct=0,error=0;
	for (int32_t i=0; i<NUM; i++)
	{
		printf("i: %d out_index_vector %d label %f [", i, data.sparse_matrix[i].vec_index, out_labels->get_label(i));

		for (index_t j=0; j<num_features; ++j)
		{
			printf("%d:%f,", data.sparse_matrix[i].features[j].feat_index,data.sparse_matrix[i].features[j].entry);
		}

		printf("]\n");
		if (i<NUM/2)
		{
			if (out_labels->get_label(i) <= 0)
			{
				correct ++;
			}
			else
			{
				error ++;
			}
		}
		else
		{
			if (out_labels->get_label(i) >= 0)
			{
				correct ++;
			}
			else
			{
				error ++;
			}
		}
	}
	printf("correct: %d error: %d \n",correct,error);

	SG_UNREF(labels);
	SG_UNREF(out_labels);
	SG_UNREF(kernel);
	SG_UNREF(features);
	SG_UNREF(svm);

	exit_shogun();
	return 0;
}
