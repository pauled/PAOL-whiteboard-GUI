#include <QtCore>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../PAOL-whiteboard-GUI/datasetimagescanner.h"
#include "../PAOL-whiteboard-GUI/PAOLProcUtils.h"

using namespace cv;

int main(int argc, char *argv[])
{
    // Set directory to save files
    string savePath = "/home/paol/shared/out/";
    string logPath = savePath + "log.log";

    // Construct stream to write to log file
    QFile logFile(QString::fromStdString(logPath));
    logFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream logStream(&logFile);

    // Initialize windows
    namedWindow("Input", CV_WINDOW_KEEPRATIO);
    namedWindow("Marker", CV_WINDOW_KEEPRATIO);

    // Initialize data set reader and current frame
    DatasetImageScanner scanner("/home/paol/shared/wboardSmall/cameraIn000000-1411752002-0.png");
    scanner.setPrintDebug(true);
    Mat currentFrame;

    for(int i = 0; i < 5; i++) {
        // Get string representation of i
        stringstream s;
        s << i;
        string countAsStr = s.str();

        // Process the next image if it exists
        if(scanner.getNextImage(currentFrame)) {
            // Show and save input image
            imshow("Input", currentFrame);
            imwrite(savePath + "input" + countAsStr + ".png", currentFrame);

            // Show and save marker image
            Mat marker = PAOLProcUtils::findMarkerWithCC(currentFrame);
            imshow("Marker", marker);
            imwrite(savePath + "marker" + countAsStr + ".png", marker);

            // Write info to log file
            logStream << "Processed image " << i << endl;

            // Needed to render the windows
            waitKey(10);
        }
    }
}
