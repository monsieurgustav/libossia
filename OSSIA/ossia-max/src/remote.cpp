// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "remote.hpp"
#include "device.hpp"
#include "parameter.hpp"
#include <ossia-max/src/utils.hpp>

#include <ossia/network/common/path.hpp>

using namespace ossia::max;

#pragma mark -
#pragma mark ossia_remote class methods

extern "C" void ossia_remote_setup()
{
  auto& ossia_library = ossia_max::instance();

  // instantiate the ossia.remote class
  ossia_library.ossia_remote_class = class_new(
      "ossia.remote", (method)ossia_remote_new, (method)ossia_remote_free,
      (short)sizeof(t_remote), 0L, A_GIMME, 0);

  if (ossia_library.ossia_remote_class)
  {
    class_addmethod(ossia_library.ossia_remote_class, (method)t_remote::remote_bind,
                    "bind", A_SYM, 0);
    // TODO why there is 2 "anything" methods ?
    class_addmethod(
        ossia_library.ossia_remote_class, (method)t_object_base::push,
        "anything", A_GIMME, 0);
    class_addmethod(
        ossia_library.ossia_remote_class, (method)t_object_base::bang, "bang",
        A_NOTHING, 0);
    class_addmethod(
        ossia_library.ossia_remote_class, (method)object_dump<t_remote>,
        "dump", A_NOTHING, 0);
    //        class_addmethod(ossia_library.ossia_remote_class,
    //        (method)ossia_remote_click,             "click",
    //        A_NOTHING,     0);

    class_addmethod(
        ossia_library.ossia_remote_class, (method)ossia_remote_assist,
        "assist", A_CANT, 0);

    class_addmethod(
        ossia_library.ossia_remote_class, (method)ossia_remote_in_bang, "bang",
        0);
    class_addmethod(
        ossia_library.ossia_remote_class, (method)ossia_remote_in_int, "int",
        A_LONG, 0);
    class_addmethod(
        ossia_library.ossia_remote_class, (method)ossia_remote_in_float,
        "float", A_FLOAT, 0);
    class_addmethod(
        ossia_library.ossia_remote_class, (method)ossia_remote_in_symbol,
        "symbol", A_SYM, 0);
    class_addmethod(
        ossia_library.ossia_remote_class, (method)t_object_base::push,
        "anything", A_GIMME, 0);

  }

  class_register(CLASS_BOX, ossia_library.ossia_remote_class);
}

extern "C" void* ossia_remote_new(t_symbol* name, long argc, t_atom* argv)
{
  auto& ossia_library = ossia_max::instance();
  t_remote* x = (t_remote*)object_alloc(ossia_library.ossia_remote_class);

  if (x)
  {
    // make outlets
    x->m_dumpout
        = outlet_new(x, NULL); // anything outlet to dump remote state

    object_obex_store(x, _sym_dumpout, (t_object*)x->m_dumpout);

    x->m_data_out = outlet_new(x, NULL); // anything outlet to output data
    x->m_set_out
        = outlet_new(x, NULL); // anything outlet to output data for ui

    new (&x->m_callbackits) decltype(x->m_callbackits);
    new (&x->m_matchers) decltype(x->m_matchers);
    x->m_dev = nullptr;

    //        x->m_clock = clock_new(x, (method)t_object_base::tick);
    x->m_regclock = clock_new(x, (method)t_object_base::bang);

    x->m_otype = object_class::remote;

    // parse arguments
    long attrstart = attr_args_offset(argc, argv);

    // check name argument
    x->m_name = _sym_nothing;
    if (attrstart && argv)
    {
      if (atom_gettype(argv) == A_SYM)
      {
        x->m_name = atom_getsym(argv);
        x->m_addr_scope = ossia::max::get_parameter_type(x->m_name->s_name);
      }
    }

    if (x->m_name == _sym_nothing)
    {
      object_error((t_object*)x, "needs a name as first argument");
      x->m_name = gensym("untitledRemote");
      return x;
    }

    // process attr args, if any
    attr_args_process(x, argc - attrstart, argv + attrstart);

    x->m_is_pattern = ossia::traversal::is_pattern(x->m_name->s_name);

    max_object_register<t_remote>(x);
    ossia_max::instance().remotes.push_back(x);
  }

  return (x);
}

extern "C" void ossia_remote_free(t_remote* x)
{
  x->m_dead = true;
  x->unregister();
  object_dequarantining<t_remote>(x);
  ossia_max::instance().remotes.remove_all(x);

  if(x->m_is_pattern && x->m_dev)
  {
    x->m_dev->on_parameter_created.disconnect<t_remote, &t_remote::on_parameter_created_callback>(x);
  }

  outlet_delete(x->m_dumpout);
  outlet_delete(x->m_set_out);
  outlet_delete(x->m_data_out);  
  x->~t_remote();
}

extern "C" void
ossia_remote_assist(t_remote* x, void* b, long m, long a, char* s)
{
  if (m == ASSIST_INLET)
  {
    sprintf(s, "I am inlet %ld", a);
  }
  else
  {
    switch(a)
    {
      case 0:
        sprintf(s, "deferred outlet with set prefix (for connecting to UI object)", a);
        break;
      case 1:
        sprintf(s, "raw outlet", a);
        break;
      case 2:
        sprintf(s, "dump outlet", a);
        break;
      default:
        break;
    }
  }
}

/*
extern "C"
void ossia_remote_click(t_remote *x, t_floatarg xpos, t_floatarg ypos,
t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{
    using namespace std::chrono;

    milliseconds ms = duration_cast< milliseconds >(
system_clock::now().time_since_epoch() );

    milliseconds diff = (ms - x->m_last_click);

    if (diff.count() < 200)
    {
        x->m_last_click = milliseconds(0);

        int l;
        t_device *device = (t_device*) find_parent(&x->m_obj, "ossia.device",
0, &l);
*/
/*
 if (!device || !x->m_node || obj_isQuarantined<t_remote>(x)){
 pd_error(x, "sorry no device found, or not connected or quarantined...");
 return;
 }
 */
/*
        t_canvas *root = x->m_obj.o_canvas;
        while (root->gl_owner)
            root = root->gl_owner;

        if (!find_and_display_friend(x, root))
            pd_error(x,"sorry I can't find a connected friend :-(");
    }
    else
        x->m_last_click = ms;
}
*/

template <typename T>
void ossia_remote_in(t_remote* x, T f)
{
  for (auto& m : x->m_matchers)
  {
    // a matcher already have valid node and address
    m.get_node()->get_parameter()->push_value(f);
  }

  if (x->m_matchers.empty())
  {
    object_error(
        (t_object*)x, "[ossia.remote %s] is not registered to any parameter",
        x->m_name->s_name);
  }
}

extern "C" void ossia_remote_in_float(t_remote* x, double f)
{
  ossia_remote_in(x, f);
}

extern "C" void ossia_remote_in_int(t_remote* x, long int f)
{
  ossia_remote_in(x, (int32_t)f);
}

extern "C" void ossia_remote_in_bang(t_remote* x)
{
  ossia_remote_in(x, ossia::impulse{});
}

extern "C" void ossia_remote_in_symbol(t_remote* x, t_symbol* f)
{
  ossia_remote_in(x, std::string(f->s_name));
}

extern "C" void ossia_remote_in_char(t_remote* x, char f)
{
  ossia_remote_in(x, f);
}

namespace ossia
{
namespace max
{

#pragma mark t_remote

bool t_remote::register_node(const std::vector<ossia::net::node_base*>& node)
{
  bool res = do_registration(node);

  if (res)
  {
    object_dequarantining<t_remote>(this);
  }
  else
    object_quarantining<t_remote>(this);

  if (!node.empty() && m_is_pattern){
    // assume all nodes refer to the same device
    auto& dev = node[0]->get_device();
    if (&dev != m_dev)
    {
      if (m_dev) m_dev->on_parameter_created.disconnect<t_remote, &t_remote::on_parameter_created_callback>(this);
      m_dev = &dev;
      m_dev->on_parameter_created.connect<t_remote, &t_remote::on_parameter_created_callback>(this);
    }
  }

  return res;
}

bool t_remote::do_registration(const std::vector<ossia::net::node_base*>& _nodes)
{
  unregister();

  std::string name = m_name->s_name;

  for (auto node : _nodes)
  {

    if (m_addr_scope == address_scope::absolute)
    {
      // get root node
      node = &node->get_device().get_root_node();
      // and remove starting '/'
      name = name.substr(1);
    }

    m_parent_node = node;

    std::vector<ossia::net::node_base*> nodes{};

    if (m_addr_scope == address_scope::global)
      nodes = ossia::max::find_global_nodes(name);
    else
      nodes = ossia::net::find_nodes(*node, name);

    for (auto n : nodes){
      if (n->get_parameter()){
        t_matcher matcher{n,this};
        m_matchers.push_back(std::move(matcher));
        m_nodes.push_back(n);
      } else {
        // if there is a node without address it might be a model
        // then look if that node have an eponyme child
        fmt::MemoryWriter path;
        path << name << "/" << name;
        auto node = ossia::net::find_node(*n, path.str());
        if (node){
          t_matcher matcher{node,this};
          m_matchers.push_back(std::move(matcher));
          m_nodes.push_back(n);
        }
      }
    }
    clock_delay(m_regclock, 0);
  }

  // do not put it in quarantine if it's a pattern
  // and even if it can't find any matching node
  return (!m_matchers.empty() || m_is_pattern);
}

bool t_remote::unregister()
{
  clock_unset(m_regclock);
  m_matchers.clear();
  m_nodes.clear();

  object_quarantining<t_remote>(this);

  m_parent_node = nullptr;
  return true;
}

void t_remote::on_parameter_created_callback(const ossia::net::parameter_base& addr)
{
  auto& node = addr.get_node();
  if (!m_name) return;
  auto path = ossia::traversal::make_path(m_name->s_name);

  // FIXME check for path validity
  if ( path && ossia::traversal::match(*path, node) )
  {
    m_matchers.emplace_back(&node,this);
  }
}

void t_remote::is_deleted(const ossia::net::node_base& n)
{
  if (!m_dead)
  {
    ossia::remove_one_if(
      m_matchers,
      [&] (const auto& m) {
        return m.get_node() == &n;
    });
  }
}

void t_remote::remote_bind(t_remote* x, t_symbol* address)
{
  x->m_name = address;
  x->m_addr_scope = ossia::max::get_parameter_type(x->m_name->s_name);
  x->unregister();
  max_object_register(x);
}

ossia::safe_vector<t_remote*>& t_remote::quarantine()
{
    return ossia_max::instance().remote_quarantine;
}


} // max namespace
} // ossia namespace
