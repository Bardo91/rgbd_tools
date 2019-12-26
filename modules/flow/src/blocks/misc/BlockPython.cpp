//---------------------------------------------------------------------------------------------------------------------
//  mico
//---------------------------------------------------------------------------------------------------------------------
//  Copyright 2018 Pablo Ramon Soria (a.k.a. Bardo91) pabramsor@gmail.com
//---------------------------------------------------------------------------------------------------------------------
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
//  and associated documentation files (the "Software"), to deal in the Software without restriction,
//  including without limitation the rights to use, copy, modify, merge, publish, distribute,
//  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or substantial
//  portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
//  OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
//  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//---------------------------------------------------------------------------------------------------------------------


#include <mico/flow/blocks/misc/BlockPython.h>

#include <flow/Policy.h>
#include <flow/Outpipe.h>
#include <flow/DataFlow.h>
#include <chrono>
#include <iostream>
#include <fstream>

#include <QtWidgets>
#include <QPushButton>

#include <mico/flow/blocks/misc/InterfaceSelectorWidget.h>

#ifdef slots
#undef slots
#endif

#include <Python.h>
#include <boost/python.hpp>
#include <boost/python/dict.hpp>

namespace mico{
    BlockPython::BlockPython(){
        // Instantiate outputs
        InterfaceSelectorWidget selectorOutputs("Outputs");
        selectorOutputs.exec();

        outputInfo_ = selectorOutputs.getInterfaces();
        for(auto &output: outputInfo_){
            createPipe(output.first, output.second);
        }

        // Instantiate Inputs
        InterfaceSelectorWidget selectorInputs("Inputs");
        selectorInputs.exec();

        inputInfo_ = selectorInputs.getInterfaces();
        if(inputInfo_.size() > 0){
            createPolicy(inputInfo_);

            std::vector<std::string> inTags;
            for(auto input: inputInfo_){
                inTags.push_back(input.first);
            }

            registerCallback(inTags, [&](flow::DataFlow _data){
                runPythonCode(_data, true);
            });
        }


        blockInterpreter_ = new QGroupBox("");
        blockInterpreterLayout_ = new QVBoxLayout();
        blockInterpreter_->setLayout(blockInterpreterLayout_);
        
        pythonEditor_ = new QTextEdit;
        highlighter_ = new PythonSyntaxHighlighter(pythonEditor_->document());
        blockInterpreterLayout_->addWidget(pythonEditor_);
        runButton_ = new QPushButton("play");
        blockInterpreterLayout_->addWidget(runButton_);
        
        QWidget::connect(runButton_, &QPushButton::clicked, [this]() {
                flow::DataFlow data({}, [](flow::DataFlow _data){});
                this->runPythonCode(data, false);
            });
    }

    void replaceAll(std::string& str, const std::string& from, const std::string& to) {
        if(from.empty())
            return;
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }

    void BlockPython::runPythonCode(flow::DataFlow _data, bool _useData){

        std::string pythonCode = pythonEditor_->toPlainText().toStdString();

    	Py_Initialize();
        PyObject* main_module = PyImport_AddModule("__main__");
        PyObject* main_dict = PyModule_GetDict(main_module);

        PyObject *locals = PyDict_New();
        if(_useData) { // Encode inputs
            for(auto input:inputInfo_){
                encodeInput(locals, _data, input.first, input.second);
            }
        }

        PyObject *pValue = PyRun_String(pythonCode.c_str(), Py_file_input, main_dict, locals);
        if (pValue == NULL) {
            PyErr_Print();
        }

        for(auto output:outputInfo_){
            flushPipe(locals, output.first, output.second);
        }
        

        Py_DECREF(pValue);
        Py_DECREF(main_module);
        Py_DECREF(main_dict);
        Py_DECREF(locals);

        Py_Finalize();
    }


    void BlockPython::encodeInput(void *_input, flow::DataFlow _data, std::string _tag, std::string _typeTag){
        PyObject *pKey = PyUnicode_FromString(_tag.c_str());
        PyObject *pValue;
        
        if(_typeTag == "int"){
            pValue = PyLong_FromLong(_data.get<int>(_tag));
        }else if(_typeTag == "float"){
            pValue = PyFloat_FromDouble(_data.get<float>(_tag));
        }else if(_typeTag == "vec3"){

        }else if(_typeTag == "vec4"){

        }else if(_typeTag == "mat33"){

        }else if(_typeTag == "mat44"){

        }else{
            std::cout << "Type " << _typeTag << " of label "<< _tag << " is not supported yet in python block." << ".It will be initialized as none. Please contact the administrators" << std::endl;
            return;
        }

        PyDict_SetItem((PyObject*) _input, pKey, pValue);
    }

    void BlockPython::flushPipe(void *_locals /*Yei...*/, std::string _tag, std::string _typeTag){
        PyObject* pValue = PyDict_GetItem((PyObject*)_locals, PyUnicode_FromString(_tag.c_str()));

        if(!pValue)  // Not filled
            return;
        
        if(_typeTag == "int"){
            getPipe(_tag)->flush((int) PyLong_AsLong(pValue));
        }else if(_typeTag == "float"){
            getPipe(_tag)->flush((float) PyFloat_AsDouble(pValue));
        }else if(_typeTag == "vec3"){

        }else if(_typeTag == "vec4"){

        }else if(_typeTag == "mat33"){

        }else if(_typeTag == "mat44"){

        }else{
            std::cout << "Type " << _typeTag << " of label "<< _tag << " is not supported yet in python block." << ".It will be initialized as none. Please contact the administrators" << std::endl;
            return;
        }
    }
}