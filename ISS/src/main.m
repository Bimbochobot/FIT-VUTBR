% Specify original image filename
filename = 'xsevci50.bmp';
% Read original image
imageOrig = imread(filename);
% Get image dimensions
dims = size(imageOrig);

%% Image sharpening
imageSharpened = imread(filename);
sharpenMatrix = [-0.5, -0.5, -0.5; -0.5, 5.0, -0.5; -0.5, -0.5, -0.5];
% Use linear filter with specified matrix
imageSharpened = imfilter(imageSharpened, sharpenMatrix);
% Save result
imwrite(imageSharpened, 'step1.bmp');

%% Image rotation
% Flip along Y axis
imageRotated = flipdim(imageSharpened, 2);
% Save result
imwrite(imageRotated, 'step2.bmp');

%% Median filter
% Use median filter with an area 5x5
imageMedian = medfilt2(imageRotated, [5 5]);
% Save result
imwrite(imageMedian, 'step3.bmp');

%% Image bluring
% Prepare bluring matrix
blurMatrix = ones(5, 5) / 49.0;
blurMatrix(2:4, 2:4) = blurMatrix(2:4, 2:4) * 3;
blurMatrix(3,3) = blurMatrix(3,3) * 3;
% Aply bluring matrix to image
imageBlured = imfilter(imageMedian, blurMatrix);
% Save result
imwrite(imageBlured, 'step4.bmp');

%% Image error created by modifications
% First, flip image back so they can be compared
imageModified = flipdim(imageBlured, 2);
% Now calculate absolute error at each pixel and sum them.
errorSum = 0.0;
for i = 1:dims(1)
    for j = 1:dims(2)
        % Pixel value has to be converted to double
        error = double(imageOrig(i,j)) - double(imageModified(i,j));
        errorSum = errorSum + abs(error);
    end
end
% Divide sum of errors by number of pixels to get average error
% Write output to console (ommiting semicolon)
errorAvg = errorSum / (dims(1) * dims(2))

%% Histrogram extending
% Show histrogram (for debugging purposes)
% imhist(imageBlured)
% Values are ~ <0, 60>, get exact values and extend histogram

% Convert image to double
imageBluredDbl = im2double(imageBlured);

% Get minimal and maximal pixel value
minPxVal = min(imageBluredDbl(:));
maxPxVal = max(imageBluredDbl(:));

% New histogram range
minHist = 0.0;
maxHist = 1.0;

% Move blured image to left, so histogram begins in x=0
imageExtHistDbl = imageBluredDbl - minPxVal;

% Calculate extension factor to fill whole histogram
extFactor = (maxHist - minHist) / (maxPxVal - minPxVal);

% Extend original histogram to new width
imageExtHistDbl = imageExtHistDbl * extFactor;

imageExtHist = im2uint8(imageExtHistDbl);
% Show extended histrogram (for debugging purposes)
% imhist(imageExtHist)

% Save result
imwrite(imageExtHist, 'step5.bmp');

%% Mean value and standard deviation calculation
% "Correct" the double histogram (introduce error, otherwise it won't
% match with example results
imageExtHistDbl = im2double(imageExtHist);

% Calculate mean values
% Output results to console (ommiting semicolon)
meanBlured = mean2(imageBluredDbl) * 255.0
meanExtHist = mean2(imageExtHistDbl) * 255.0

% Calculate standard deviations
% Output results to console (ommiting semicolon)
stdBlured = std2(imageBluredDbl) * 255.0
stdExtHist = std2(imageExtHistDbl) * 255.0

%% Image quantization
N = 2;
a = 0.0;
b = 1.0;
imageQuantified = zeros(dims);
for i = 1:dims(1)
    for j = 1:dims(2)
        imageQuantified(i, j) = ...
        round(((2^N)-1)*(imageExtHistDbl(i, j)-a)/(b-a))*(b-a)/((2^N)-1)+a;
    end
end
imwrite(imageQuantified, 'step6.bmp');

