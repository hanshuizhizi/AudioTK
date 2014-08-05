/**
 * \file TypedBaseFilter.cpp
 */

#include "TypedBaseFilter.h"

#include <cstdint>

#include <boost/mpl/distance.hpp>
#include <boost/mpl/empty.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/utility/enable_if.hpp>

#include "Utilities.h"

namespace
{
  typedef boost::mpl::vector<std::int16_t, std::int32_t, std::int64_t, float, double> ConversionTypes;

  template<typename Vector, typename DataType>
  typename boost::enable_if<typename boost::mpl::empty<Vector>::type, void>::type
  convert_array(ATK::BaseFilter* filter, int port, DataType* converted_input, std::int64_t size, int type)
  {
  }

  template<typename Vector, typename DataType>
  typename boost::disable_if<typename boost::mpl::empty<Vector>::type, void>::type
      convert_array(ATK::BaseFilter* filter, int port, DataType* converted_input, std::int64_t size, int type)
  {
    if(type != 0)
    {
      convert_array<typename boost::mpl::pop_front<Vector>::type, DataType>(filter, port, converted_input, size, type - 1);
    }
    else
    {
      typedef typename boost::mpl::front<Vector>::type InputOriginalType;
      InputOriginalType* original_input_array = static_cast<ATK::TypedBaseFilter<InputOriginalType>*>(filter)->get_output_array(port);
      ATK::ConversionUtilities<InputOriginalType, DataType>::convert_array(original_input_array, converted_input, size);
    }
  }
}

namespace ATK
{
  template<typename DataType>
  TypedBaseFilter<DataType>::TypedBaseFilter(int nb_input_ports, int nb_output_ports)
  :Parent(nb_input_ports, nb_output_ports), converted_inputs_delay(nb_input_ports), converted_inputs(nb_input_ports, NULL), converted_inputs_size(nb_input_ports, 0), outputs_delay(nb_output_ports), outputs(nb_output_ports, NULL), outputs_size(nb_output_ports, 0)
  {
  }

  template<typename DataType>
  TypedBaseFilter<DataType>::~TypedBaseFilter()
  {
  }
  
  template<typename DataType>
  void TypedBaseFilter<DataType>::set_nb_input_ports(int nb_ports)
  {
    if(nb_ports == nb_input_ports)
      return;
    Parent::set_nb_input_ports(nb_ports);
    std::vector<boost::scoped_array<DataType> > temp(nb_ports);
    converted_inputs_delay.swap(temp);
    converted_inputs.assign(nb_ports, NULL);
    converted_inputs_size.assign(nb_ports, 0);
  }
  
  template<typename DataType>
  void TypedBaseFilter<DataType>::set_nb_output_ports(int nb_ports)
  {
    if(nb_ports == nb_output_ports)
      return;
    Parent::set_nb_output_ports(nb_ports);
    std::vector<boost::scoped_array<DataType> > temp(nb_ports);
    outputs_delay.swap(temp);
    outputs.assign(nb_ports, NULL);
    outputs_size.assign(nb_ports, 0);
  }

  template<typename DataType>
  void TypedBaseFilter<DataType>::process_impl(std::int64_t size)
  {
  }

  template<typename DataType>
  void TypedBaseFilter<DataType>::prepare_process(std::int64_t size)
  {
    convert_inputs(size);
  }

  template<typename DataType>
  int TypedBaseFilter<DataType>::get_type() const
  {
    return boost::mpl::distance<boost::mpl::begin<ConversionTypes>::type, typename boost::mpl::find<ConversionTypes, DataType>::type >::value;
  }

  template<typename DataType>
  DataType* TypedBaseFilter<DataType>::get_output_array(int port)
  {
    return outputs[port];
  }

  template<typename DataType>
  void TypedBaseFilter<DataType>::convert_inputs(std::int64_t size)
  {
    for(int i = 0; i < nb_input_ports; ++i)
    {
      if(input_delay <= connections[i].second->get_output_delay() && connections[i].second->get_type() == get_type())
      {
        converted_inputs[i] = reinterpret_cast<TypedBaseFilter<DataType>* >(connections[i].second)->get_output_array(connections[i].first);
        continue;
      }
      if(converted_inputs_size[i] < size)
      {
        boost::scoped_array<DataType> temp(new DataType[input_delay + size]);
        if(converted_inputs_size[i] == 0)
        {
          for(int j = 0; j < input_delay; ++j)
          {
            temp[j] = 0;
          }
        }
        else
        {
          for(int j = 0; j < input_delay; ++j)
          {
            temp[j] = converted_inputs[i][converted_inputs_size[i] + j - input_delay];
          }
        }
        
        converted_inputs_delay[i].swap(temp);
        converted_inputs[i] = converted_inputs_delay[i].get() + input_delay;
        converted_inputs_size[i] = size;
      }
      else
      {
        for(int j = 0; j < input_delay; ++j)
        {
          converted_inputs[i][j - input_delay] = converted_inputs[i][converted_inputs_size[i] + j - input_delay];
        }
      }
      convert_array<ConversionTypes, DataType>(connections[i].second, connections[i].first, converted_inputs[i], size, connections[i].second->get_type());
    }
  }
  
  template<typename DataType>
  void TypedBaseFilter<DataType>::prepare_outputs(std::int64_t size)
  {
    for(int i = 0; i < nb_output_ports; ++i)
    {
      if(outputs_size[i] < size)
      {
        boost::scoped_array<DataType> temp(new DataType[output_delay + size]);
        if(outputs_size[i] == 0)
        {
          for(int j = 0; j < output_delay; ++j)
          {
            temp[j] = 0;
          }
        }
        else
        {
          for(int j = 0; j < output_delay; ++j)
          {
            temp[j] = outputs[i][outputs_size[i] + j - output_delay];
          }
        }
        
        outputs_delay[i].swap(temp);
        outputs[i] = outputs_delay[i].get() + output_delay;
        outputs_size[i] = size;
      }
      else
      {
        for(int j = 0; j < output_delay; ++j)
        {
          outputs[i][j - output_delay] = outputs[i][outputs_size[i] + j - output_delay];
        }
      }
    }
  }

  template<typename DataType>
  void TypedBaseFilter<DataType>::full_setup()
  {
    // Reset input arrays
    std::vector<boost::scoped_array<DataType> > temp_in(nb_input_ports);
    converted_inputs_delay.swap(temp_in);
    converted_inputs.assign(nb_input_ports, NULL);
    converted_inputs_size.assign(nb_input_ports, 0);

    // Reset output arrays
    std::vector<boost::scoped_array<DataType> > temp_out(nb_output_ports);
    outputs_delay.swap(temp_out);
    outputs.assign(nb_output_ports, NULL);
    outputs_size.assign(nb_output_ports, 0);

    Parent::full_setup();
  }

  template class TypedBaseFilter<std::int16_t>;
  template class TypedBaseFilter<std::int32_t>;
  template class TypedBaseFilter<std::int64_t>;
  template class TypedBaseFilter<float>;
  template class TypedBaseFilter<double>;
}