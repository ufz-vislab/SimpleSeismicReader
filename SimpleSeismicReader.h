#ifndef __SimpleSeismicReader_h
#define __SimpleSeismicReader_h

#include "vtkImageAlgorithm.h"

#include <vector>

class SimpleSeismicReader : public vtkImageAlgorithm
{
public:
	vtkTypeMacro(SimpleSeismicReader,vtkImageAlgorithm);
	void PrintSelf(ostream& os, vtkIndent indent);

	static SimpleSeismicReader *New();

	// Description:
	// Specify file name of the .abc file.
	vtkSetStringMacro(FileName);
	vtkGetStringMacro(FileName);

protected:
	SimpleSeismicReader();
	~SimpleSeismicReader(){}

	vtkTimeStamp ReadHeaderTime;

	float depthStart;
	float depthStep;
	float depthSamples;

	float x_origin;
	float y_origin;
	float x_spacing;
	float y_spacing;
	int xy_dim;

	std::vector<float> ReadLine(std::string line);
	int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
	int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

	//vtkDelimitedTextReader* Reader;

private:
	SimpleSeismicReader(const SimpleSeismicReader&);  // Not implemented.
	void operator=(const SimpleSeismicReader&);  // Not implemented.

	char* FileName;
};

#endif
