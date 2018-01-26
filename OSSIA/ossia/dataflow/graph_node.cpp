// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <ossia/dataflow/graph_node.hpp>
#include <ossia/dataflow/dataflow.hpp>
#include <ossia/dataflow/fx_node.hpp>
#include <ossia/dataflow/execution_state.hpp>
#include <ossia/dataflow/dataflow.hpp>

namespace ossia
{
graph_edge::graph_edge(connection c, std::size_t pout, std::size_t pin, node_ptr pout_node, node_ptr pin_node)
  : graph_edge{c, pout_node->outputs()[pout], pin_node->inputs()[pin], std::move(pout_node) , std::move(pin_node)}
{
}

graph_edge::graph_edge(connection c, outlet_ptr pout, inlet_ptr pin, node_ptr pout_node, node_ptr pin_node)
  : con{c}
  , out{std::move(pout)}
  , in{std::move(pin)}
  , out_node{std::move(pout_node)}
  , in_node{std::move(pin_node)}
{
}

void graph_edge::init()
{
  if (in && out)
  {
    out->connect(this);
    in->connect(this);

    if (auto delay = con.target<delayed_glutton_connection>())
    {
      ossia::apply(init_delay_line{delay->buffer}, out->data);
    }
    else if (auto sdelay = con.target<delayed_strict_connection>())
    {
      ossia::apply(init_delay_line{sdelay->buffer}, out->data);
    }
  }
}

graph_edge::~graph_edge()
{
  clear();
}

void graph_edge::clear()
{
  if (in && out)
  {
    out->disconnect(this);
    in->disconnect(this);
  }

  con = connection{};
  out.reset();
  in.reset();
  out_node.reset();
  in_node.reset();
}

graph_node::~graph_node()
{

}
graph_node::graph_node()
{
}

bool graph_node::consumes(const execution_state&) const
{
  return false;
}

void graph_node::run(token_request t, execution_state&)
{
}

std::string graph_node::label() const
{
  return {};
}

bool graph_node::has_port_inputs() const
{
  return ossia::any_of(
        inputs(), [](const inlet_ptr& inlet) { return !inlet->sources.empty(); });
}

bool graph_node::has_global_inputs() const
{
  return ossia::any_of(inputs(), [&](const inlet_ptr& inlet) {
    return (inlet->scope & port::scope_t::global) && bool(inlet->address);
  });
}

bool graph_node::has_local_inputs(const execution_state& st) const
{
  return ossia::any_of(inputs(), [&] (const inlet_ptr& inlet) {
    if (inlet->scope & port::scope_t::local)
    {
      bool b = false;

      // TODO optimize by stopping when found
      // TODO valueDevices -> generalize
      apply_to_destination(inlet->address, st.allDevices,
                           [&] (ossia::net::parameter_base* addr, bool) {
        if (!b || st.in_local_scope(*addr))
          b = true;
      });

      if(b)
        return true;

      if (consumes(st))
        return true;
    }
    return false;
  });
}


void graph_node::clear()
{
  m_inlets.clear();
  m_outlets.clear();
}

void graph_node::all_notes_off() { }
}
