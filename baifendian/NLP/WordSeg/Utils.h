//============================================================================
// Name        : Utils.h
// Created on  : Jul 23, 2010
// Author      : Zhijin GUO
// Copyright   : Baifendian Information Technology
// Description :
//============================================================================


#ifndef UTILS_H_
#define UTILS_H_

// system library header file
#include <iterator>
#include <algorithm>

using namespace std;

namespace std {
	template <typename T1, typename T2>
    ostream & operator << (ostream & os_, const pair<T1, T2> & pair_value )
    {
        return os_ << pair_value.first << "\t" << pair_value.second;
    }

	template <typename T>
	ostream & operator << (ostream & os_, vector<T> & vec_t) {
		typename vector<T>::const_iterator citr_begin = vec_t.begin();
		typename vector<T>::const_iterator citr_end = vec_t.end();
		copy(citr_begin, citr_end, ostream_iterator<typename T::value_type>(cout, "\t"));
		return cout;
	}

//	template <typename Key, typename Value>
//	ostream & operator << (ostream & os_, tr1::unordered_map<Key, Value> & vec_t) {
//		typename tr1::unordered_map<Key, Value>::const_iterator citr_begin = vec_t.begin();
//		typename tr1::unordered_map<Key, Value>::const_iterator citr_end = vec_t.end();
//		copy(citr_begin, citr_end, ostream_iterator<pair<Key, Value> >(cout, "\t"));
//		return cout;
//	}

}

namespace baifendian_common {

	template <class T>
	void Print(T & container) {
		typename T::const_iterator citr_begin = container.begin();
		typename T::const_iterator citr_end = container.end();
		copy(citr_begin, citr_end, ostream_iterator<typename T::value_type>(cout, "\t"));
		cout << endl;
	}

	template <class T>
	void Print(const T & container) {
		typename T::const_iterator citr_begin = container.begin();
		typename T::const_iterator citr_end = container.end();
		copy(citr_begin, citr_end, ostream_iterator<typename T::value_type>(cout, "\t"));
		cout << endl;
	}

	template <class T1, class T2>
	void Print(T1 & container1, T2 & container2) {
		typename T1::const_iterator citr1_begin = container1.begin();
		typename T1::const_iterator citr1_end = container1.end();
		typename T2::const_iterator citr2_begin = container2.begin();
		typename T2::const_iterator citr2_end = container2.end();
		copy(citr1_begin, citr1_end, ostream_iterator<typename T1::value_type>(cout, "\t"));
		copy(citr2_begin, citr2_end, ostream_iterator<typename T2::value_type>(cout, "\t"));
		cout << endl;
	}


	template<class T>
	class Singleton {
	public:
		static T& createInstance() {
			if(s_pInstance==NULL)
				s_pInstance=new T();
			return *s_pInstance;
		}
		static void deleteInstance(){
			delete s_pInstance;
			s_pInstance=NULL;
		}
	protected:
		Singleton(void) {
		}
		virtual ~Singleton(void) {
		}
		Singleton(const Singleton<T>&);
		Singleton<T> & operator=(const Singleton<T> &);
	private:
		static T * s_pInstance;
	};

	// initialize instance static variable
	template <class T>
	T * Singleton<T>::s_pInstance=NULL;

	class TestSingleton : public Singleton<TestSingleton> {
	public:
	private:
		TestSingleton();
		~TestSingleton();
		friend class Singleton<TestSingleton>;
	};

	// 求两个排序集合的交集，结果存储在set中
	template<typename InputIterator1, typename InputIterator2, typename SetType>
	void setIntersect(InputIterator1 first1, InputIterator1 last1,
					  InputIterator2 first2, InputIterator2 last2, set<SetType> & result) {
		while (first1 != last1 && first2 != last2) {
			if (*first1 < *first2) {
				++first1;
			} else if (*first2 < *first1) {
				++first2;
			} else 	{
				result.insert(*first1);
				++first1;
				++first2;
			}
		}
	}
} // end of namespace baifendian_common

#endif /* UTILS_H_ */
