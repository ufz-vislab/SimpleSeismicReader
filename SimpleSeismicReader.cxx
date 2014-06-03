#include "SimpleSeismicReader.h"

#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkDataObject.h"
#include "vtkSmartPointer.h"
#include "vtkImageData.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

vtkStandardNewMacro(SimpleSeismicReader);

SimpleSeismicReader::SimpleSeismicReader()
{
	this->depthStart = 0;
	this->depthStep = 0;
	this->depthSamples = 0;
	this->FileName = NULL;
	this->SetNumberOfInputPorts(0);
	this->SetNumberOfOutputPorts(1);
}

int SimpleSeismicReader::RequestData(
	vtkInformation *vtkNotUsed(request),
	vtkInformationVector **vtkNotUsed(inputVector),
	vtkInformationVector *outputVector)
{

	// get the info object
	vtkInformation *outInfo = outputVector->GetInformationObject(0);

	// get the ouptut
	vtkImageData *output = vtkImageData::SafeDownCast(
		outInfo->Get(vtkDataObject::DATA_OBJECT()));

	output->SetExtent(
		outInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()));
	output->AllocateScalars(outInfo);

	if(!this->FileName)
	{
		vtkErrorMacro(<< "A FileName must be specified.");
		return -1;
	}

	std::ifstream in (this->FileName, std::ios::in);
	if(!in.is_open())
	{
		vtkErrorMacro(<< "File " << this->FileName << " not found");
		return -1;
	}

	vtkDebugMacro (<< "reading seismic header");

	// Header info are 3 values in the first line
	std::string line_string;
	getline(in, line_string);
	std::vector<float> vals = this->ReadLine(line_string);
	if(vals.size() != 3)
	{
		vtkErrorMacro(<< "File " << this->FileName << " header info invalid!");
		delete &vals;
		return -1;
	}
	depthStart = vals[0];
	depthStep = vals[1];
	depthSamples = vals[2];
	//delete &vals;

	// Calculate spacing from line 2 and 3
	getline(in, line_string);
	std::vector<float> line2_vals = this->ReadLine(line_string);
	getline(in, line_string);
	std::vector<float> line3_vals = this->ReadLine(line_string);

	if(line2_vals.size() < 2 || line3_vals.size() < 2)
	{
		vtkErrorMacro(<< "File " << this->FileName << " data invalid!");
		//delete &line2_vals;
		//delete &line3_vals;
		return -1;
	}

	x_origin = line3_vals[0];
	y_origin = line3_vals[1];
	x_spacing = line3_vals[0] - line2_vals[0];
	y_spacing = line3_vals[1] - line2_vals[1];

	std::size_t lines_count = 3;
	while (std::getline(in, line_string))
		++lines_count;
	xy_dim = lines_count - 1;

	//delete &line2_vals;
	//delete &line3_vals;

	//vtkSmartPointer<vtkImageData> pxImage = vtkSmartPointer<vtkImageData>::New();
	output->SetDimensions(xy_dim, xy_dim, depthSamples);
	output->SetOrigin(x_origin, y_origin, depthStart);
	output->SetSpacing(x_spacing, y_spacing, depthStep);
	//pxImage->SetScalarTypeToUnsignedChar();
	//pxImage->SetNumberOfScalarComponents(1);
	//pxImage->AllocateScalars();

	in.close();

	//output->ShallowCopy(pxImage);

	return 1;
}

std::vector<float>& SimpleSeismicReader::ReadLine(std::string line)
{
	std::stringstream str;
	str << line;
	std::vector<float>* values = new std::vector<float>();
	std::string cell;
	while(std::getline(str, cell, '\t'))
	{
		std::istringstream ss(cell);
		float f;
		if(!(ss >> f))
		{
			vtkErrorMacro(<< "Value could not be converted to float");
		}
		values->push_back(f);
	}

	return *values;
}

void SimpleSeismicReader::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os,indent);

	os << indent << "File Name: "
			<< (this->FileName ? this->FileName : "(none)") << "\n";
}
