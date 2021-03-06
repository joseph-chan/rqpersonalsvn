/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 2012 Heiko Strathmann
 */
#include <shogun/labels/BinaryLabels.h>

using namespace shogun;

void test_sigmoid_fitting()
{
	CBinaryLabels* labels=new CBinaryLabels(10);
	labels->set_confidences(SGVector<float64_t>(labels->get_num_labels()));

	for (index_t i=0; i<labels->get_num_labels(); ++i)
		labels->set_confidence(i%2==0 ? 1 : -1, i);

	labels->get_confidences().display_vector("scores");
	labels->scores_to_probabilities();

	/* only two probabilities will be the result, repeatedly,
	 * assert against reference implementation */
	ASSERT(CMath::abs(labels->get_confidence(0)-0.8571428439385661)<10E-15);
	ASSERT(CMath::abs(labels->get_confidence(1)-0.14285715606143384)<10E-15);

	SG_UNREF(labels);
}

int main()
{
	init_shogun_with_defaults();

//	sg_io->set_loglevel(MSG_DEBUG);

	test_sigmoid_fitting();

	exit_shogun();
	return 0;
}

