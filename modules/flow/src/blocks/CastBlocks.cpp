
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


#include <mico/flow/blocks/CastBlocks.h>
#include <flow/Outpipe.h>

namespace mico{
    
    BlockDataframeToSomething::BlockDataframeToSomething(){
        createPolicy({{{"Dataframe", "dataframe"}}});

        registerCallback({"Dataframe"}, 
                                [&](flow::DataFlow _data){
                                        if(idle_){
                                            idle_ = false;
                                                auto df = _data.get<mico::Dataframe<pcl::PointXYZRGBNormal>::Ptr>("Dataframe");  
                                                getPipe(tagToGet())->flush(dataToget(df));
                                            idle_ = true;
                                        }
                                    }
                                );
    }


    std::string BlockDataframeToPose::name() {
        return "Dataframe -> Pose";
    }
    
    BlockDataframeToPose::BlockDataframeToPose(){ 
        createPipe("Pose","mat44"); 
    }
    
    std::any BlockDataframeToPose::dataToget(mico::Dataframe<pcl::PointXYZRGBNormal>::Ptr &_df){
        return _df->pose();
    };
    
    std::string BlockDataframeToPose::tagToGet() {
        return "pose";
    };


}
