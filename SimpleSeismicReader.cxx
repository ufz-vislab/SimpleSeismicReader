#include "SimpleSeismicReader.h"

#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkDataObject.h"
#include "vtkSmartPointer.h"
#include "vtkImageData.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"

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

int SimpleSeismicReader::RequestInformation(
	vtkInformation *vtkNotUsed(request),
	vtkInformationVector **vtkNotUsed(inputVector),
	vtkInformationVector *outputVector)
{
	vtkInformation *outInfo = outputVector->GetInformationObject(0);

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
	xy_dim = sqrt(lines_count - 1);

	in.close();

	int ext[6] = {0, xy_dim, 0, xy_dim, 0, depthSamples};
	double spacing[3] = {x_spacing, y_spacing, depthStep};
	double origin[3] = {0, 0, 0};
	outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), ext, 6);
	outInfo->Set(vtkDataObject::SPACING(), spacing, 3);
	outInfo->Set(vtkDataObject::ORIGIN(), origin, 3);
	vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_FLOAT, 1);

	return 1;
}

int SimpleSeismicReader::RequestData(
	vtkInformation *vtkNotUsed(request),
	vtkInformationVector **vtkNotUsed(inputVector),
	vtkInformationVector *outputVector)
{
	vtkInformation *outInfo = outputVector->GetInformationObject(0);
	vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_FLOAT, 1);

	// get the ouptut
	vtkImageData *output = vtkImageData::SafeDownCast(
		outInfo->Get(vtkDataObject::DATA_OBJECT()));

	output->SetExtent(
		outInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()));
	output->AllocateScalars(outInfo);

	//delete &line2_vals;
	//delete &line3_vals;

	//vtkSmartPointer<vtkImageData> pxImage = vtkSmartPointer<vtkImageData>::New();
	output->SetDimensions(xy_dim, xy_dim, depthSamples);
	//output->SetOrigin(x_origin, y_origin, depthStart);
	output->SetOrigin(0, 0, depthStart);
	output->SetSpacing(x_spacing, y_spacing, depthStep);
	output->AllocateScalars(VTK_FLOAT, 1);
	vtkDataArray* scalars = output->GetPointData()->GetScalars();
	scalars->SetName("Data");

	// skip first line
	std::ifstream in (this->FileName, std::ios::in);
	std::string line_string;
	getline(in, line_string);
	std::size_t current_line = 0;

	while (std::getline(in, line_string))
	{
		std::vector<float> vals = this->ReadLine(line_string);

		for(int z = 0; z < depthSamples; z++)
			output->SetScalarComponentFromFloat((int)(current_line / xy_dim), current_line % (xy_dim), z, 0, vals[2 + z]);
		++current_line;
	}

	in.close();


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
