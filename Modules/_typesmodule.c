/* _types module: provides the Buffer type */

#ifndef Py_BUILD_CORE_BUILTIN
#  define Py_BUILD_CORE_MODULE 1
#endif

#include "Python.h"
#include "pycore_moduleobject.h"  // _PyModule_GetState()

typedef struct {
    PyTypeObject *BufferMetaType;
    PyTypeObject *BufferType;
} _types_state;

static PyObject *
buffermeta_instancecheck(PyObject *self, PyObject *instance)
{
    if (PyObject_CheckBuffer(instance)) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject *
buffermeta_subclasscheck(PyObject *self, PyObject *instance)
{
    if (!PyType_Check(instance)) {
        PyErr_SetString(PyExc_TypeError, "issubclass() arg 1 must be a class");
        return NULL;
    }
    PyTypeObject *type = (PyTypeObject *)instance;
    if (type->tp_as_buffer != NULL && type->tp_as_buffer->bf_getbuffer != NULL) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyMethodDef buffermeta_methods[] = {
    {"__instancecheck__", buffermeta_instancecheck, METH_O},
    {"__subclasscheck__", buffermeta_subclasscheck, METH_O},
    {0}
};

static PyType_Slot buffermeta_slots[] = {
    {Py_tp_methods, buffermeta_methods},
    {0, NULL},
};

static PyType_Spec buffermeta_spec = {
    .name = "_types.BufferMeta",
    .flags = (Py_TPFLAGS_DEFAULT | Py_TPFLAGS_DISALLOW_INSTANTIATION |
              Py_TPFLAGS_IMMUTABLETYPE),
    .slots = buffermeta_slots,
};

static PyType_Slot buffer_slots[] = {
    {0, NULL},
};

static PyType_Spec buffer_spec = {
    .name = "_types.Buffer",
    .flags = (Py_TPFLAGS_DEFAULT | Py_TPFLAGS_DISALLOW_INSTANTIATION |
              Py_TPFLAGS_IMMUTABLETYPE),
    .slots = buffer_slots,
};

static int
_types_traverse(PyObject *module, visitproc visit, void *arg)
{
    _types_state *state = (_types_state *)_PyModule_GetState(module);
    Py_VISIT(state->BufferType);
    Py_VISIT(state->BufferMetaType);
    return 0;
}

static int
_types_clear(PyObject *module)
{
    _types_state *state = (_types_state *)_PyModule_GetState(module);
    Py_CLEAR(state->BufferType);
    Py_CLEAR(state->BufferMetaType);
    return 0;
}

static void
_types_free(void *module)
{
    _types_clear((PyObject *)module);
}

static int
_types_modexec(PyObject *m)
{
    _types_state *state = (_types_state *)_PyModule_GetState(m);
    state->BufferMetaType = (PyTypeObject *)PyType_FromModuleAndSpec(
        m, &buffermeta_spec, (PyObject *)&PyType_Type
    );
    if (state->BufferMetaType == NULL) {
        return -1;
    }
    state->BufferType = (PyTypeObject *)PyType_FromModuleAndSpec(m, &buffer_spec, NULL);
    if (state->BufferType == NULL) {
        return -1;
    }
    Py_INCREF((PyObject *)state->BufferMetaType);
    Py_XDECREF(((PyObject *)state->BufferType)->ob_type);
    ((PyObject *)state->BufferType)->ob_type = state->BufferMetaType;
    Py_INCREF((PyObject *)state->BufferType);
    if (PyModule_AddObject(m, "Buffer", (PyObject *)state->BufferType) < 0) {
        Py_DECREF((PyObject *)state->BufferType);
        return -1;
    }
    return 0;
}

static PyModuleDef_Slot _typesmodule_slots[] = {
    {Py_mod_exec, _types_modexec},
    {0, NULL}
};

PyDoc_STRVAR(_types_doc,
"Implementation of types.Buffer.\n");

static struct PyModuleDef typesmodule = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "_types",
    .m_size = sizeof(_types_state),
    .m_doc = _types_doc,
    .m_slots = _typesmodule_slots,
    .m_traverse = _types_traverse,
    .m_clear = _types_clear,
    .m_free = _types_free,
};

PyMODINIT_FUNC
PyInit__types(void)
{
    return PyModuleDef_Init(&typesmodule);
}
