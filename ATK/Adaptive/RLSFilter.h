/**
 * \file RLSFilter.h
 */

#ifndef ATK_ADAPTIVE_RLSFILTER_H
#define ATK_ADAPTIVE_RLSFILTER_H

#include <ATK/Core/TypedBaseFilter.h>
#include "config.h"

namespace ATK
{
  /// RLS implementation as a filter
  template<typename DataType_>
  class ATK_ADAPTIVE_EXPORT RLSFilter : public TypedBaseFilter<DataType_>
  {
    class RLSFilterImpl;
    std::unique_ptr<RLSFilterImpl> impl;
  protected:
    typedef TypedBaseFilter<DataType_> Parent;
    using typename Parent::DataType;
    using Parent::converted_inputs;
    using Parent::outputs;
    using Parent::nb_input_ports;
    using Parent::nb_output_ports;
    using Parent::input_delay;

  public:
    /**
     * @brief Creates the filter with a given size
     * @param size is the size of the underlying MA filter
     */
    RLSFilter(std::size_t size);
    /// Destructor
    ~RLSFilter();
    
    /// Changes the underlying size
    void set_size(std::size_t size);
    /// Retrieve the size
    std::size_t get_size() const;

    /// Sets the starting P matrix
    void set_P(const DataType_* P);
    /// Retrieves P
    const DataType_* get_P() const;
    /// Sets the starting w matrix
    void set_w(const DataType_* w);
    /// Retrieves w
    const DataType_* get_w() const;

    /// Sets the memory of the RLS algorithm
    void set_memory(DataType_ memory);
    /// Retrieves the memory
    DataType_ get_memory() const;

    /// Sets the learning mode
    void set_learning(bool learning);
    /// Am I in learning mode or not?
    bool get_learning() const;

  protected:
    virtual void process_impl(std::size_t size) const;
    
  private:
    std::size_t global_size;
    bool learning;
  };
}

#endif
