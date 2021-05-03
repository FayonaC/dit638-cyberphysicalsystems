/*
 * Copyright (C) 2020  Christian Berger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Include the single-file, header-only middleware libcluon to create high-performance microservices
#include "cluon-complete.hpp"
#include "cluon-complete.cpp"

// Include the OpenDLV Standard Message Set that contains messages that are usually exchanged for automotive or robotic applications 
#include "opendlv-standard-message-set.hpp"

// Include the GUI and image processing header files from OpenCV
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

int32_t main(int32_t argc, char **argv) {
    int32_t retCode{1};
    // Parse the command line parameters as we require the user to specify some mandatory information on startup.
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
    if ( (0 == commandlineArguments.count("cid")) ||
         (0 == commandlineArguments.count("name")) ||
         (0 == commandlineArguments.count("width")) ||
         (0 == commandlineArguments.count("height")) ) {
        std::cerr << argv[0] << " attaches to a shared memory area containing an ARGB image." << std::endl;
        std::cerr << "Usage:   " << argv[0] << " --cid=<OD4 session> --name=<name of shared memory area> [--verbose]" << std::endl;
        std::cerr << "         --cid:    CID of the OD4Session to send and receive messages" << std::endl;
        std::cerr << "         --name:   name of the shared memory area to attach" << std::endl;
        std::cerr << "         --width:  width of the frame" << std::endl;
        std::cerr << "         --height: height of the frame" << std::endl;
        std::cerr << "Example: " << argv[0] << " --cid=253 --name=img --width=640 --height=480 --verbose" << std::endl;
    }
    else {
        // Extract the values from the command line parameters
        const std::string NAME{commandlineArguments["name"]};
        const uint32_t WIDTH{static_cast<uint32_t>(std::stoi(commandlineArguments["width"]))};
        const uint32_t HEIGHT{static_cast<uint32_t>(std::stoi(commandlineArguments["height"]))};
        const bool VERBOSE{commandlineArguments.count("verbose") != 0};

        // Attach to the shared memory.
        std::unique_ptr<cluon::SharedMemory> sharedMemory{new cluon::SharedMemory{NAME}};
        if (sharedMemory && sharedMemory->valid()) {
            std::clog << argv[0] << ": Attached to shared memory '" << sharedMemory->name() << " (" << sharedMemory->size() << " bytes)." << std::endl;

            // Interface to a running OpenDaVINCI session where network messages are exchanged.
            // The instance od4 allows you to send and receive messages.
            cluon::OD4Session od4{static_cast<uint16_t>(std::stoi(commandlineArguments["cid"]))};

            opendlv::proxy::GroundSteeringRequest gsr;
            std::mutex gsrMutex;
            auto onGroundSteeringRequest = [&gsr, &gsrMutex](cluon::data::Envelope &&env){
                // The envelope data structure provide further details, such as sampleTimePoint as shown in this test case:
                // https://github.com/chrberger/libcluon/blob/master/libcluon/testsuites/TestEnvelopeConverter.cpp#L31-L40
                std::lock_guard<std::mutex> lck(gsrMutex);
                gsr = cluon::extractMessage<opendlv::proxy::GroundSteeringRequest>(std::move(env));
                //std::cout << "lambda: groundSteering = " << gsr.groundSteering() << std::endl;
            };

            od4.dataTrigger(opendlv::proxy::GroundSteeringRequest::ID(), onGroundSteeringRequest);
 
// HSV values
 int minHueBlue = 102;
 int maxHueBlue = 150;
 int minSatBlue = 88;
 int maxSatBlue = 165;
 int minValueBlue = 43;
 int maxValueBlue = 222;

 int minHueYellow = 0;
 int maxHueYellow = 46;
 int minSatYellow  = 108;
 int maxSatYellow = 221;
 int minValueYellow = 104;
 int maxValueYellow = 255;


            // Endless loop; end the program by pressing Ctrl-C.
            while (od4.isRunning()) {
                // OpenCV data structure to hold an image.
                cv::Mat img;

                // Wait for a notification of a new frame.
                sharedMemory->wait();

                // Lock the shared memory.
                sharedMemory->lock();
                {
                    // Copy the pixels from the shared memory into our own data structure.
                    cv::Mat wrapped(HEIGHT, WIDTH, CV_8UC4, sharedMemory->data());
                    img = wrapped.clone();
                }
                // TODO: Here, you can add some code to check the sampleTimePoint when the current frame was captured.                            

                // Get current sample time
                // this is a data type that holds 2 in
                std::pair<bool, cluon::data::TimeStamp> sTime = sharedMemory->getTimeStamp(); // Saving current time in sTime var

               // Convert TimeStamp obj into microseconds
               	int64_t sMicro = cluon::time::toMicroseconds(sTime.second);

                char buffer[25];
                std::sprintf(buffer, "ts: %ld; ", sMicro); 
                //std::cout << buffer;

                sharedMemory->unlock();

                // TODO: Do something with the frame.
                // Example: Draw a red rectangle and display image.
                cv::rectangle(img, cv::Rect(350, 245, 230, 115), cv::Scalar(255,255,0));
                cv::rectangle(img, cv::Rect(125, 245, 230, 115), cv::Scalar(0,0,255));
                cv::rectangle(img, cv::Rect(200, 245, 230, 115), cv::Scalar(255,255,255));

                // Cutting a region of interest
		        // centered cv::Rect(200, 245, 230, 115)
                cv::Rect regionOfInterestYellow = cv::Rect(350, 245, 230, 115);
                cv::Rect regionOfInterestBlue = cv::Rect(125, 245, 230, 115);
                cv::Rect regionOfInterestCentre = cv::Rect(200, 245, 230, 115);

                cv::Mat imageWithRegionYellow = img(regionOfInterestYellow);
                cv::Mat imageWithRegionBlue = img(regionOfInterestBlue);
                cv::Mat imageWithRegionCentre = img(regionOfInterestCentre);

                // Operation to find blue cones in HSV image
		// maybe good for blue cv::Rect(125, 245, 230, 115)
                cv::Mat hsvBlueImg;
                cv::cvtColor(imageWithRegionBlue, hsvBlueImg, cv::COLOR_BGR2HSV);
                cv::Mat detectBlueImg;
                cv::inRange(hsvBlueImg, cv::Scalar(minHueBlue, minSatBlue, minValueBlue), cv::Scalar(maxHueBlue, maxSatBlue, maxValueBlue), detectBlueImg);

            //Applying Gaussian blur to detectBlueImg
                cv::GaussianBlur(detectBlueImg, detectBlueImg, cv::Size(5, 5), 0);

            //Applying dilate and erode to detectBlueImg to remove holes from foreground
                cv::dilate(detectBlueImg, detectBlueImg, 0);
                cv::erode(detectBlueImg, detectBlueImg, 0);

            //Applying erode and dilate to detectBlueImg to remove small objects from foreground
                cv::erode(detectBlueImg, detectBlueImg, 0);
                cv::dilate(detectBlueImg, detectBlueImg, 0);

            // The below will find the contours of the cones in detectBlueImg and store them in a vector
                std::vector<std::vector<cv::Point> > blueContours;
                std::vector<cv::Vec4i> blueHierarchy;
                cv::findContours(detectBlueImg, blueContours, blueHierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

            // The below will draw the cone contours onto detectBlueImg (copied code from opencv doc)
                int idx = 0;
                cv::Mat blueContourImage = cv::Mat::zeros(detectBlueImg.rows, detectBlueImg.cols, CV_8UC3);
                for( ; idx >=0; idx = blueHierarchy[idx][0]) {
                    cv::Scalar colour( 255, 255, 0);
                    // 2 draws the outline of the cones. Replacing this with -1 would fill in the cone shape
                    cv::drawContours(blueContourImage, blueContours, idx, colour, 2, 8, blueHierarchy );
                }

		 // Operation to find yellow cones in HSV image
	         // cv::Rect(300, 245, 230, 115) "good" for yellow
               cv::Mat hsvYellowImg;
                cv::cvtColor(imageWithRegionYellow, hsvYellowImg, cv::COLOR_BGR2HSV);
                cv::Mat detectYellowImg;
                cv::inRange(hsvYellowImg, cv::Scalar(minHueYellow, minSatYellow, minValueYellow), cv::Scalar(maxHueYellow, maxSatYellow, maxValueYellow), detectYellowImg);

             //Applying Gaussian blur to detectYellowImg
                cv::GaussianBlur(detectYellowImg, detectYellowImg, cv::Size(5, 5), 0);

             //Applying dilate and erode to detectYellowImg to remove holes from foreground
                cv::dilate(detectYellowImg, detectYellowImg, 0);
                cv::erode(detectYellowImg, detectYellowImg, 0);

             //Applying erode and dilate to detectYellowImg to remove small objects from foreground
                cv::erode(detectYellowImg, detectYellowImg, 0);
                cv::dilate(detectYellowImg, detectYellowImg, 0);

            // The below will find the contours of the cones in detectYellowImg and store them in a vector
                std::vector<std::vector<cv::Point> > yellowContours;
                std::vector<cv::Vec4i> yellowHierarchy;
                cv::findContours(detectYellowImg, yellowContours, yellowHierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

            // The below will draw the cone contours onto detectYellowImg (copied code from opencv doc)
                int idx2 = 0;
                cv::Mat yellowContourImage = cv::Mat::zeros(detectYellowImg.rows, detectYellowImg.cols, CV_8UC3);
                for( ; idx2 >=0; idx2 = yellowHierarchy[idx2][0]) {
                    cv::Scalar colour( 255, 255, 0);
                    // 2 draws the outline of the cones. Replacing this with -1 would fill in the cone shape
                    cv::drawContours(yellowContourImage, yellowContours, idx2, colour, 2, 8, yellowHierarchy );
                }

                // Add current UTC time
                // Ref: https://stackoverflow.com/questions/38686405/convert-time-t-from-localtime-zone-to-utc   
               	cluon::data::TimeStamp time = cluon::time::now(); // Saves current time to var
                int sec = time.seconds(); // Saves current time as int to sec var
                std::time_t lt = sec; // Initialize time_t using sec var
                char buf[30]; // Buffer to hold time
                auto utc_field = *std::gmtime(&lt); // Converts local time_t to UTC tm, auto deduces type

                // Formats buffer results into string
                std::strftime(buf, sizeof(buf), "Now: %FT%TZ; ", &utc_field); 

                // Prints current time in terminal
                //std::cout << buf; 

                // Concatonating three items into one complete string
                std::string name = "Group 16"; // name
                std::string total = std::string(buf).append(buffer); // Concatonates ms and UTC
                std::string complete = total.append(std::string(name)); // Concats all info

                // Displays information on video
                cv::putText(img, //target image
                			complete, 
                    		cv::Point(25, 50), 
                    		cv::FONT_HERSHEY_DUPLEX,
                    		0.5,
                    		CV_RGB(0,250,154));

                                {
                    std::lock_guard<std::mutex> lck(gsrMutex);
                    std::cout << "group_16;" << " sampleTimeStamp in microseconds: " << sMicro << " steeringWheelAngle: " << gsr.groundSteering() << std::endl;
                }

                // Display image on your screen.
                if (VERBOSE) {
                    cv::imshow(sharedMemory->name().c_str(), blueContourImage);
		    //cv::imshow(sharedMemory->name().c_str(), yellowContourImage);
                    cv::waitKey(1);
                }
            }
        }
        retCode = 0;
    }
    return retCode;
}
