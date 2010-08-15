#ifndef FACTOR_TEMPLATE_H
#define FACTOR_TEMPLATE_H

#include "gm.h"

BEGIN_GRAPHICAL_MODEL_NAMESPACE;

/**
  * A function class which is described by a value table
  * and implemented as a map from variable assignment to double or any number_type
  * that is capable of =, +, +=, -, -=, *, *=, /, /=. Example:
  for (TableF<double>::iterator it = f.begin(); it != f.end(); it++)
  {
    const Assignment& assignment = it->first;
    TableF::factor_value_type value = it->second; // double
  }
  */
template <typename _V>
class TableF : public Map<Assignment, _V, AssignmentCompare>
{
  TableF() {}  // hide the default constructor
public:
  typedef _V factor_value_type;

  /** Generate all possible assignments for a domain that agree with restrict assignment */
  TableF(const Domain& dom, const Assignment& res = Assignment());

  /** remove assignments that do not agree with variable in a assignment */
  void restricted(const Assignment& a);

  /** For operator[](a), the assignment must be in the current set of assignments
    * otherwise, the result is a double reference to 0.0
    */
  factor_value_type& operator[](const Assignment& a);

  /** For get(a), the assignment could be a superset of the domain */
  factor_value_type get(const Assignment& a);

  /** Return the domain of this factor */
  const Domain& domain();

  void print(const std::string& name = "") const;
protected:
  static factor_value_type trash;
  Domain dom_;

  /** Helper function to generate all assignments agree with res within the domain */
  void genAssignments(const Domain& dom, unsigned int index, const Assignment& res, Assignment& temp);
};

template <typename _V> typename TableF<_V>::factor_value_type TableF<_V>::trash = 0.0;

template <typename _V>
void TableF<_V>::genAssignments(const Domain& dom, unsigned int index, const Assignment& res, Assignment& temp)
{
  // check if all variables has value
  if (index == dom.size())
  {
    if (index > 0)
      insert(typename TableF::value_type(temp, typename TableF::factor_value_type(1.0)));
    return;
  }
  // populate all values of variable dom[index] that agree with res
  const Variable* var = (const Variable*) dom[index];
  Assignment::const_iterator it = res.find(var);
  if (it != res.end())
  {
    int val = (*it).second;
    DEBUG_ASSERT(val >= 0 && val < var->cardinality());
    temp[var] = val;
    genAssignments(dom, index+1, res, temp);
  }
  else
  {
    for (int val = 0; val < var->cardinality(); val++)
    {
      temp[var] = val;
      genAssignments(dom, index+1, res, temp);
    }
  }
}

// Generate all possible assignments for a domain that agree with restrict assignment
template <typename _V>
TableF<_V>::TableF(const Domain& dom, const Assignment& res) : dom_(dom)
{
  Assignment temp;
  for (unsigned int i = 0; i < dom.size(); i++)
    DEBUG_ASSERT(dom[i]->type() == VARIABLE_FINITE);
  genAssignments(dom, 0, res, temp);
}

// remove assignments that do not agree with variable in a assignment
template <typename _V>
void TableF<_V>::restricted(const Assignment& a)
{
  Vector<typename TableF::iterator> its;
  for (typename TableF::iterator it = this->begin(); it != this->end(); it++)
  {
    const Assignment& b = it->first;
    if (!b.agree(a)) {
      //        b.print("erase");
      its << it;
    }
  }
  for (typename Vector<typename TableF::iterator>::iterator it = its.begin(); it != its.end(); it++)
    this->erase(*it);
}

template <typename _V>
typename TableF<_V>::factor_value_type& TableF<_V>::operator[](const Assignment& a)
{
  trash = typename TableF::factor_value_type(0.0);
  typename TableF::iterator it = this->find(a);
  // a.print("[]");
  // cout << "not found = " << (it == this->end()) << endl;
  return it == this->end() ? trash : (*it).second;
}

template <typename _V>
const Domain& TableF<_V>::domain() { return dom_; }

template <typename _V>
void TableF<_V>::print(const std::string& name) const
{
  cout << name; if (!name.empty()) cout << " = " << endl;
  for (typename TableF::const_iterator it = this->begin(); it != this->end(); it++)
  {
    const gm::Assignment& a = it->first;
    const factor_value_type&  val = it->second;
    cout << val << " <-- "; a.print();
  }
}

template <typename _V>
typename TableF<_V>::factor_value_type TableF<_V>::get(const Assignment& a)
{
  // check if dom is a subset of variables in a
  Assignment temp;
  for (unsigned int i = 0; i < dom_.size(); i++)
  {
    Assignment::const_iterator it = a.find(dom_[i]);
    if (it == a.end()) return 0.0;
    else temp[dom_[i]] = (*it).second;
  }
  return this->operator [](temp);
}

END_GRAPHICAL_MODEL_NAMESPACE;
#endif // FACTOR_TEMPLATE_H
