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


#include <fstream>
#include <mico/base/object_detection/ml/classification/BoW.h>
#include <mico/base/object_detection/ml/SlidingWindow.h>
#include <boost/program_options.hpp>

#include <opencv2/videoio/videoio_c.h>
#include <opencv2/highgui/highgui_c.h>

using namespace mico;
using namespace cv;
using namespace std;
using namespace boost::program_options;

//---------------------------------------------------------------------------------------------------------------------
void singleImageMultipleObject(BoW _objDetector, cv::Mat _image, cv::Mat &_result, cv::Mat  &_probs);
void filterPredictions(vector<vector<Rect>> &_predictions);

//---------------------------------------------------------------------------------------------------------------------
int main(int _argc, char ** _argv) {


	options_description desc{"Options"};
    desc.add_options()
      ("help,h", "Help screen")
      ("alpha", value<float>()->default_value(0.5), "Alpha hiperparameter for diritchled distribution of LDA model")
      ("beta", value<float>()->default_value(1), "Beta hiperparameter for diritchled distribution of LDA model")
      ("train_images", value<std::string>()->default_value(""), "Pattern to train images")
      ("gt", value<std::string>()->default_value(""), "Groundtruth")
      ("n_topics", value<int>()->required(), "Number of labels")
	  ("n_words", value<int>()->required(), "Number of words")
	  ("eval_images", value<std::string>()->default_value(""), "Pattern to eval images")
      ("mode", value<std::string>()->required(), "train|eval")
      ("model_name", value<std::string>()->default_value("optimized_model"), "Name of output/input model")
      ("multitrain", "If set, it will train several models");


	// //--------------------
	// options_description descHelp{"help_options"};
    // descHelp.add_options()
    //   ("help,h", "Help screen");

    // variables_map vmHelp;
    // store(parse_command_line(_argc, _argv, descHelp), vmHelp);
    // notify(vmHelp);

	// if (vmHelp.count("help")){
	// 	std::cout << desc << '\n';
	// 	return 0;
	// }
	// //--------------------

    variables_map vm;
    store(parse_command_line(_argc, _argv, desc), vm);
    notify(vm);

	// Create and train detector.
	  int nTopics = vm["n_topics"].as<int>();
	unsigned int nWords = vm["n_words"].as<int>();


	if(vm["mode"].as<std::string>() == "train"){
		std::cout << "Training mode" << std::endl;
		if(vm.count("multitrain")){
			std::cout << "Multitrain selected" << std::endl;
			std::vector<int> trainWords = {100,200,300,500,700};
			std::vector<float> alphas = {0.1,0.3,0.7,1,5,10,20};
			std::vector<float> betas = {0.1,0.3,0.7,1,5,10,20};

			for(auto words: trainWords){
				for(auto alpha:alphas){
					for(auto beta: betas){
						std::cout << "Training. Words " << words << ", alpha " << alpha << ", beta " << beta << std::endl;
						
						BoW::Params params = {300, 1, 1, BoW::Params::eExtractorType::SIFT, BoW::Params::eDescriptorType::SIFT, unsigned(words) };
						BoW objDetector;
						objDetector.params(params);
						
						LdaModel ldaModel;
						ldaModel.setParams(alpha, beta);
						objDetector.model(ldaModel);
						objDetector.train(	vm["train_images"].as<std::string>(),
											vm["gt"].as<std::string>());
						objDetector.save("model_a"+std::to_string(int(10*alpha))+"_b"+std::to_string(int(10*beta))+"_w"+std::to_string(words));
					}
				}
			}
			
		}else{
			BoW::Params params = {300, 1, 1, BoW::Params::eExtractorType::SIFT, BoW::Params::eDescriptorType::SIFT, nWords };
			BoW objDetector;
			objDetector.params(params);
			LdaModel ldaModel;
			ldaModel.setParams(vm["alpha"].as<float>(), vm["beta"].as<float>());
			objDetector.model(ldaModel);

			std::cout << "Training" << std::endl;
			objDetector.train(	vm["train_images"].as<std::string>(),
								vm["gt"].as<std::string>());
			objDetector.save(vm["model_name"].as<std::string>());
			std::cout << "Trained" << std::endl;
		}
	}else if(vm["mode"].as<std::string>() == "eval"){
		BoW::Params params = {300, 1, 1, BoW::Params::eExtractorType::SIFT, BoW::Params::eDescriptorType::SIFT, nWords };
		BoW objDetector;
		objDetector.params(params);


		LdaModel ldaModel;
		ldaModel.setParams(vm["alpha"].as<float>(), vm["beta"].as<float>());
		objDetector.model(ldaModel);
		objDetector.load(vm["model_name"].as<std::string>());
		std::cout << "Loaded model" << std::endl;
	
		cv::VideoCapture imageStream(vm["eval_images"].as<std::string>());
		cv::VideoWriter resultVideo(	"result_"+std::to_string(time(NULL))+".avi",
										CV_FOURCC('M','J','P','G'), 
										20,
										cv::Size(300,300));
		cv::VideoWriter classesVideo(	"classes_"+std::to_string(time(NULL))+".avi",
										CV_FOURCC('M','J','P','G'), 
										20,
										cv::Size(300*2,300*2));
		if (!classesVideo.isOpened() || !classesVideo.isOpened()){
			cout  << "Could not open the output videos for write" << endl;
			return -1;
		}
		for (;;) {
			Mat frame, res, probs;
			imageStream >> frame;
			if (frame.rows != 0) {
				resize(frame, frame, Size(300,300));
				singleImageMultipleObject(objDetector, frame, res, probs);
				resultVideo << res;
				classesVideo << probs;
			}
			else {
				break;
			}
		}
		resultVideo.release();
		classesVideo.release();
	}
}
int wIndex = 0;
//---------------------------------------------------------------------------------------------------------------------
void singleImageMultipleObject(BoW _objDetector,  cv::Mat _image, cv::Mat &_result, cv::Mat  &_probs) {
	Mat dis;
	std::vector<cv::Rect> grid;
	_image.copyTo(dis);

	SlidingWindow::Params paramsSw = { 50,50, 25, 25, 1, 0.5 };
	SlidingWindow sw(paramsSw);
	grid = sw.grid(_image);

	std::vector<std::pair<unsigned, float>> topics = _objDetector.evaluate(_image, grid);
	
	std::vector<cv::Scalar> colors = {cv::Scalar(255,0,0),cv::Scalar(0,255,0),cv::Scalar(0,0,255),cv::Scalar(255,0,255)};
	Mat display = _image.clone();
	cv::Mat finDis1 = _image.clone(), 
	finDis2 = _image.clone(), 
	finDis3 = _image.clone(), 
	finDis4 = _image.clone();

	if(topics.size() == 0)
		return;

	vector<vector<Rect>> predictions(4);
	for (unsigned i = 0; i < grid.size(); i++) {
		if (topics[i].second > 0.9) {
			// Store rectangles with higher probabilities
			predictions[topics[i].first].push_back(grid[i]);
		}
	}

	for (unsigned i = 0; i < grid.size(); i++) {
		int thickness = topics[i].second*5;

		switch (topics[i].first) {
		case 0:
			cv::rectangle(finDis1, grid[i], colors[topics[i].first], thickness);
			break;
		case 1:
			cv::rectangle(finDis2, grid[i], colors[topics[i].first], thickness);
			break;
		case 2:
			cv::rectangle(finDis3, grid[i], colors[topics[i].first], thickness);
			break;
		case 3:
			cv::rectangle(finDis4, grid[i], colors[topics[i].first], thickness);
			break;
		}
	}

	filterPredictions(predictions);

	for (unsigned topic = 0; topic < predictions.size(); topic++) {
		for (unsigned i = 0; i <predictions[topic].size(); i++) {
				cv::rectangle(display, predictions[topic][i], colors[topic], 3);
		}
	}

	hconcat(finDis1,finDis2,finDis1);
	hconcat(finDis3,finDis4,finDis3);
	vconcat(finDis1,finDis3,finDis1);

	_result = display;
	_probs = finDis1;
	cv::imshow("display", display);
	waitKey(3);
}

void filterPredictions(vector<vector<Rect>>& _predictions) {
	for (vector<Rect> &list : _predictions) {
		for (unsigned int i = 0; i < list.size(); i++) {
			for (unsigned int j = i+1; j < list.size(); j++) {
				double areaUnion = (list[i]&list[j]).area();
				double areaDifference = list[i].area() + list[j].area() - areaUnion;

				if (areaUnion / areaDifference > 0.14) {

					Rect aux = list[j] | list[i];
					list.erase(list.begin() + j);
					list.erase(list.begin() + i);
					list.push_back(aux);
					i = -1;
					break;
				}
			}
		}
	}
}
