#include "/usr/local/lib/python3.8/dist-packages/tensorflow/include/external/local_config_python/python_include/Python.h"
#include <stdlib.h>

extern void Initialize_Game();

PyObject* f_Initialize_Game(PyObject* self, PyObject* args)
{
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    Initialize_Game();
    return Py_BuildValue("");
}


extern int game_proceed(int side, char* instruction);

PyObject* f_game_proceed(PyObject* self, PyObject* args)
{
    int side;
    char* instruction=calloc(10, sizeof(char));
    int display_mode;
    int result;
    if (!PyArg_ParseTuple(args, "is", &side, &instruction))
        return NULL;
    result=game_proceed(side, instruction);
    fflush(stdout);
    return Py_BuildValue("i", result);
}


extern void Display_Board(int side);

PyObject* f_Display_Board(PyObject* self, PyObject* args)
{
    int side;

    
    if (!PyArg_ParseTuple(args, "i", &side))
        return NULL;
    Display_Board(side);
    fflush(stdout);
    return Py_BuildValue("");
}


extern char* Get_Info();

PyObject* f_Get_Info(PyObject* self, PyObject* args)
{
    char* info;
    
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    info=Get_Info();
    return Py_BuildValue("s", info);
}


extern char* Alpha_Beta(int side);

PyObject* f_Alpha_Beta(PyObject* self, PyObject* args)
{
    char* order;
    int side;
    
    if(!PyArg_ParseTuple(args, "i", &side))
        return NULL;
    order=Alpha_Beta(side);
    fflush(stdout);
    return Py_BuildValue("s", order);
}


static PyMethodDef methods[] = {
    {"Initialize_Game", f_Initialize_Game, METH_VARARGS},
    {"Game_Proceed", f_game_proceed, METH_VARARGS},
    {"Display_Board", f_Display_Board, METH_VARARGS},
    {"Get_Info", f_Get_Info, METH_VARARGS},
    {"Alpha_Beta", f_Alpha_Beta, METH_VARARGS},
    {NULL}
};


static struct PyModuleDef Gogo_Shogi =
{
    PyModuleDef_HEAD_INIT,
    "Gogo_Shogi",
    "",
    -1,
    methods
};


PyMODINIT_FUNC PyInit_Gogo_Shogi(void)
{
    return PyModule_Create(&Gogo_Shogi);
}
