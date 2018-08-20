//---------------------------------------------------------------------------------------------------------------------
//  RGBD_TOOLS
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

namespace rgbd{
    //---------------------------------------------------------------------------------------------------------------------
    template <typename PointType_, DebugLevels DebugLevel_, OutInterfaces OutInterface_>
    inline BundleAdjuster_g2o<PointType_, DebugLevel_, OutInterface_>::BundleAdjuster_g2o() {
        // Init optimizer
        #ifdef USE_G2O
           

        #endif
    }


    //---------------------------------------------------------------------------------------------------------------------
    template <typename PointType_, DebugLevels DebugLevel_, OutInterfaces OutInterface_>
    inline void BundleAdjuster_g2o<PointType_, DebugLevel_, OutInterface_>::appendCamera(int _id, Eigen::Matrix4f _pose, cv::Mat _intrinsics = cv::Mat(), cv::Mat _distcoeff = cv::Mat()){
        #ifdef USE_G2O
            if(_id == 0){ // Assuming IDs starts always from 0
                double focal_length = _intrinsics.at<float>(0,0);
                Eigen::Vector2d principal_point(_intrinsics.at<float>(0,2), 
                                                _intrinsics.at<float>(1,2)    );
    
                g2o::CameraParameters * cam_params = new g2o::CameraParameters(focal_length, principal_point, 0.);
                cam_params->setId(0);

                if (!mOptimizer->addParameter(cam_params)) {
                    assert(false);
                }
            }

            int vertexID = mCurrentGraphID;
            mCameraId2GraphId[_id] = vertexID;
            mCurrentGraphID++;

            // Camera vertex 
            g2o::VertexSE3Expmap * v_se3 = new g2o::VertexSE3Expmap();
            v_se3->setId(vertexID);


            Eigen::Vector3d trans = _pose.block<3,1>(0,3).cast<double>();
            Eigen::Quaterniond q(_pose.block<3,3>(0,0).cast<double>());
            g2o::SE3Quat pose(q,trans);

            v_se3->setEstimate(pose);

            if (vertexID < 2)
                v_se3->setFixed(true);

            mOptimizer->addVertex(v_se3);
        #endif
    }

    //---------------------------------------------------------------------------------------------------------------------
    template <typename PointType_, DebugLevels DebugLevel_, OutInterfaces OutInterface_>
    inline void BundleAdjuster_g2o<PointType_, DebugLevel_, OutInterface_>::appendPoint(int _id, Eigen::Vector3f _position){
        #ifdef USE_G2O
            int pointID = mCurrentGraphID;
            mPointId2GraphId[_id] = pointID;
            mCurrentGraphID++;

            g2o::VertexSBAPointXYZ * v_p = new g2o::VertexSBAPointXYZ();

            v_p->setId(_id);
            v_p->setMarginalized(true);
            v_p->setEstimate(_position.cast<double>());

            mOptimizer->addVertex(v_p);
        #endif
    }

    //---------------------------------------------------------------------------------------------------------------------
    template <typename PointType_, DebugLevels DebugLevel_, OutInterfaces OutInterface_>
    inline void BundleAdjuster_g2o<PointType_, DebugLevel_, OutInterface_>::appendProjection(int _idCamera, int _idPoint, cv::Point2f _projection){
        #ifdef USE_G2O
            // 66 G2O does not handle distortion, there are two options, undistort points always outside or do it just here. But need to define it properly!
            g2o::EdgeProjectXYZ2UV * e = new g2o::EdgeProjectXYZ2UV();

            Eigen::Vector2d z(_projection.x, _projection.y);
            e->setMeasurement(z);
            e->information() = Eigen::Matrix2d::Identity();

            e->vertices()[0] = dynamic_cast<g2o::OptimizableGraph::Vertex*>(mOptimizer->vertices().find(_idPoint)->second);
            e->vertices()[1] = dynamic_cast<g2o::OptimizableGraph::Vertex*>(mOptimizer->vertices().find(_idCamera)->second);
            // Not sure what is this, but adding allways.
            g2o::RobustKernelHuber* rk = new g2o::RobustKernelHuber;
            e->setRobustKernel(rk);
            
            e->setParameterId(0, 0);    // Set camera params

            mOptimizer->addEdge(e);
        #endif
    }

    //---------------------------------------------------------------------------------------------------------------------
    template <typename PointType_, DebugLevels DebugLevel_, OutInterfaces OutInterface_>
    inline void BundleAdjuster_g2o<PointType_, DebugLevel_, OutInterface_>::reserveData(int _cameras, int _words){

    }

    //---------------------------------------------------------------------------------------------------------------------
    template <typename PointType_, DebugLevels DebugLevel_, OutInterfaces OutInterface_>
    inline void BundleAdjuster_g2o<PointType_, DebugLevel_, OutInterface_>::fitSize(int _cameras, int _words){

    }

    //---------------------------------------------------------------------------------------------------------------------
    template <typename PointType_, DebugLevels DebugLevel_, OutInterfaces OutInterface_>
    inline void BundleAdjuster_g2o<PointType_, DebugLevel_, OutInterface_>::cleanData(){
        #ifdef USE_G2O
            if(mOptimizer != nullptr)
                delete mOptimizer;
            
            mOptimizer = new g2o::SparseOptimizer;

            mOptimizer->setVerbose(true);
            
            std::unique_ptr<g2o::BlockSolver_6_3::LinearSolverType> linearSolver = g2o::make_unique<g2o::LinearSolverCholmod<g2o::BlockSolver_6_3::PoseMatrixType>>();
            // std::unique_ptr<g2o::BlockSolver_6_3::LinearSolverType> linearSolver = g2o::make_unique<g2o::LinearSolverCSparse<g2o::BlockSolver_6_3::PoseMatrixType>>();
            
            mSolver = new g2o::OptimizationAlgorithmLevenberg(g2o::make_unique<g2o::BlockSolver_6_3>(std::move(linearSolver)));
            mOptimizer->setAlgorithm(mSolver);

            mPointId2GraphId.clear();
            mCameraId2GraphId.clear();
            mCurrentGraphID = 0;

        #endif
    }

    //---------------------------------------------------------------------------------------------------------------------
    template <typename PointType_, DebugLevels DebugLevel_, OutInterfaces OutInterface_>
    inline void BundleAdjuster_g2o<PointType_, DebugLevel_, OutInterface_>::checkData(){

    }

    //---------------------------------------------------------------------------------------------------------------------
    template <typename PointType_, DebugLevels DebugLevel_, OutInterfaces OutInterface_>
    inline bool BundleAdjuster_g2o<PointType_, DebugLevel_, OutInterface_>::doOptimize(){
        #ifdef USE_G2O
            mOptimizer->initializeOptimization();
            mOptimizer->save("g2o_graph"+std::to_string(time(NULL))+".g2o");
            return mOptimizer->optimize(this->mBaIterations);
        #else
            return false;
        #endif
    }

    //---------------------------------------------------------------------------------------------------------------------
    template <typename PointType_, DebugLevels DebugLevel_, OutInterfaces OutInterface_>
    inline void BundleAdjuster_g2o<PointType_, DebugLevel_, OutInterface_>::recoverCameras(){
        #ifdef USE_G2O
            for(unsigned idx = 0; idx < this->mCameraId2GraphId.size(); idx++){
                int id = this->mCameraId2GraphId[idx];

                g2o::VertexSE3Expmap * v_se3 = dynamic_cast< g2o::VertexSE3Expmap * > (mOptimizer->vertex(id));
                //if(v_se3 != nullptr){         //Removed condition   666 THIS IS GOOD!, isn't it?? 666 666 666
                    //std::cout << "Pose of df: " << frameId.first << std::endl << mClusterframe->poses[frameId.first] << std::endl;
                    g2o::SE3Quat pose;
                    pose = v_se3->estimate();
                        
                    auto cluster = this->mClusterFrames[id]; 

                    Eigen::Matrix4f newPose = pose.to_homogeneous_matrix().cast<float>();
                    cluster->bestDataframePtr()->updatePose(newPose);
                    Eigen::Matrix4f offsetCluster = cluster->bestDataframePtr()->pose.inverse()*newPose;

                    for(auto &df : cluster->dataframes){
                        if(df.second->id != cluster->bestDataframe){
                            Eigen::Matrix4f updatedPose = offsetCluster*df.second->pose;
                            df.second->updatePose(updatedPose);
                        }
                    }
                //}
            }
        #endif
    }

    //---------------------------------------------------------------------------------------------------------------------
    template <typename PointType_, DebugLevels DebugLevel_, OutInterfaces OutInterface_>
    inline void BundleAdjuster_g2o<PointType_, DebugLevel_, OutInterface_>::recoverPoints(){
        #ifdef USE_G2O
            for(unsigned idx = 0; idx < this->mWordIdxToId.size(); idx++){
                int id = this->mPointId2GraphId[idx];

                g2o::VertexSBAPointXYZ * v_p= dynamic_cast< g2o::VertexSBAPointXYZ * > (mOptimizer->vertex(id));
                Eigen::Vector3d p_pos = v_p->estimate();
                this->mGlobalUsedWordsRef[id]->point = {
                    (float) p_pos[0],
                    (float) p_pos[1],
                    (float) p_pos[2]
                };
                this->mGlobalUsedWordsRef[id]->optimized = true;
            }
        #endif
    }
}