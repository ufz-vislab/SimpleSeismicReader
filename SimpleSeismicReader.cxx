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

	if (!this->FileName)
	{
		vtkErrorMacro(<< "A FileName must be specified.");
		return 0;
	}

	if (output->GetScalarType() != VTK_FLOAT)
	{
		vtkErrorMacro("Execute: This source only outputs floats.");
		return 1;
	}

//
// Read header
//
	if (this->ReadHeader () == 0)
	{
		//
		// Read Profiles
		//
		this->ReadData (output);
	}

	return 1;
}

std::vector<std::string>& SimpleSeismicReader::ReadLine(std::string line)
{
	std::stringstream str;
	str << line;
	std::vector<std::string>* values = new std::vector<std::string>();
	std::string cell;
	while(std::getline(str, cell, '\t'))
		values->push_back(cell);

	return *values;
}

int SimpleSeismicReader::ReadHeader()
{
	if(this->GetMTime() < this->ReadHeaderTime)
		return 0;

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

	std::string line_string;
	getline(in, line_string);
	this->ReadLine(line_string);

	this->ReadHeaderTime.Modified();
	in.close();
	return 0;
}

int SimpleSeismicReader::ReadData(vtkImageData *data)
{

}

void SimpleSeismicReader::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os,indent);

	os << indent << "File Name: "
			<< (this->FileName ? this->FileName : "(none)") << "\n";
}
