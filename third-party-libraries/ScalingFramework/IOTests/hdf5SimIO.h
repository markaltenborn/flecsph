#ifndef _HDF5_SIM_IO_H
#define _HDF5_SIM_IO_H

#include "simIO.h"
#include "hdf5.h"
#include "H5hut.h"

#include <iostream>
#include <mpi.h>

namespace Flecsi_Sim_IO
{

// 
// HDF5 Represenation

// Database: file
// Timestep: Group
//      Each Variable: Dataset
//          Datatype: datatype
//          Type: Dataset attribute (point or grid or ...)
//                Dataspace (for strunctured grid description with dims ...)
//          
//      Timestep : Group attribute
//      Timestamp: Group attribute


class HDF5SimIO: public SimIO
{
    h5_file_t *dataFile;

  public:
    HDF5SimIO():SimIO(){}
    HDF5SimIO(std::string _outputFileName):SimIO(_outputFileName){ dataFile=NULL; }

    hid_t getHDF5Datatype(std::string _datatypeName);

    int createDataset(OutputType _datasetType, int _numVars, int _numDims, MPI_Comm _comm);
    int openFile(std::string filename, MPI_Comm _comm);
    void closeFile(){ H5CloseFile(dataFile); }
    void setTimeStep(int ts){ H5SetStep(dataFile, ts); }


    template <class T>
    int writeGlobalAttribute(std::string _name, std::string _dataType, T _data, int numElements=1);

    template <class T>
    int writeTimestepAttribute(std::string _name, std::string _dataType, T _data, int numElements=1);

    int writeGridData();
    int writePointData();
    int writePointData(int ts);
    int writePointData(int ts, MPI_Comm _comm);
};
// https://support.hdfgroup.org/HDF5/doc1.8/RM/RM_H5Front.html



int HDF5SimIO::createDataset(OutputType _datasetType, int _numVars, int _numDims, MPI_Comm _comm)
{ 
    datasetType = _datasetType;  
    numVars = _numVars;  
    numDims = _numDims; 

    dataFile = H5OpenFile(outputFileName.c_str(), H5_O_RDWR, _comm);
    if (!dataFile)
        return -1;

    return 0;
}


int HDF5SimIO::openFile(std::string filename, MPI_Comm  _comm)
{
    dataFile = H5OpenFile(filename.c_str(), H5_O_RDWR, _comm);
    if (!dataFile)
        return -1;

    return 0;
}





hid_t HDF5SimIO::getHDF5Datatype(std::string _datatypeName)
{
    hid_t _datatype;
    if (_datatypeName == "double")
        _datatype = H5Tcopy(H5T_NATIVE_DOUBLE);
    else if (_datatypeName == "int")
        _datatype = H5Tcopy(H5T_NATIVE_INT);
    else if (_datatypeName == "float")
        _datatype = H5Tcopy(H5T_NATIVE_FLOAT);
    else if (_datatypeName == "char")
        _datatype = H5Tcopy(H5T_NATIVE_CHAR);
    else
    {
        std::cout << "Datatype " << _datatypeName << " has not been defined yet!" << std::endl;
        _datatype = -1;
    }

    return _datatype;
}





template <class T>
inline int HDF5SimIO::writeGlobalAttribute(std::string _name, std::string _dataType, T _data, int numElements)
{
    // if ( _dataType == "string" )
    // {
    //     H5WriteFileAttribString(dataFile, _name.c_str(), _data);
    // }
    // else 
    if ( _dataType == "int32_t" )
    {
        int32_t data = (int32_t)_data;
        H5WriteFileAttribInt32(dataFile, _name.c_str(), &data, 1);
    }
    else if ( _dataType == "int64_t" )
    {
        int64_t data = (int64_t)_data;
        H5WriteFileAttribInt64(dataFile, _name.c_str(), &data, 1);
    }
    else if ( _dataType == "float" )
    {
        float data = (float)_data;
        H5WriteFileAttribFloat32(dataFile, _name.c_str(), &data, 1);
    }
    else if ( _dataType == "double" )
    {
        double data = (double)_data;
        H5WriteFileAttribFloat64(dataFile, _name.c_str(), &data, 1);
    }
}





inline int HDF5SimIO::writePointData(int ts, MPI_Comm _comm)
{
    for (int i=0; i<timestepAttributes.size(); i++)
    {
        if ( timestepAttributes[i].dataType == "int32_t" )
            H5WriteStepAttribInt32(dataFile, (timestepAttributes[i].name).c_str(), (int32_t *)timestepAttributes[i].data, timestepAttributes[i].numElements);
        else if ( timestepAttributes[i].dataType == "int64_t" )
            H5WriteStepAttribInt64(dataFile, (timestepAttributes[i].name).c_str(), (int64_t *)timestepAttributes[i].data, timestepAttributes[i].numElements);
        else if ( timestepAttributes[i].dataType == "float" )
            H5WriteStepAttribFloat32(dataFile, (timestepAttributes[i].name).c_str(), (float *)timestepAttributes[i].data, timestepAttributes[i].numElements);
        else if ( timestepAttributes[i].dataType == "double" )
            H5WriteStepAttribFloat64(dataFile, (timestepAttributes[i].name).c_str(), (double *)timestepAttributes[i].data, timestepAttributes[i].numElements);
        else
            return -1;
    }
    
    for (int i=0; i<vars.size(); i++)
    {
        H5PartSetNumParticles(dataFile, vars[i].numElements);
        
        if ( vars[i].dataType == "float" )
            H5PartWriteDataFloat32(dataFile, (vars[i].name).c_str(), (float *)vars[i].data);
        else if ( vars[i].dataType == "double" )
            H5PartWriteDataFloat64(dataFile, (vars[i].name).c_str(), (double *)vars[i].data);
        else if ( vars[i].dataType == "int32_t" )
            H5PartWriteDataInt32(dataFile, (vars[i].name).c_str(), (int32_t *)vars[i].data);
        else if ( vars[i].dataType == "int64_t" )
            H5PartWriteDataInt64(dataFile, (vars[i].name).c_str(), (int64_t *)vars[i].data);
        else
            return -1;
    }

    timestepAttributes.clear();
    vars.clear();

    return 0;
}




template <class T>
inline int HDF5SimIO::writeTimestepAttribute(std::string _name, std::string _dataType, T _data, int numElements)
{
    // if ( _dataType == "string" )
    // {
    //     H5WriteFileAttribString(dataFile, _name.c_str(), _data);
    // }
    // else 
    if ( _dataType == "int32_t" )
    {
        int32_t data = (int32_t)_data;
        H5WriteStepAttribInt32(dataFile, _name.c_str(), &data, 1);
    }
    else if ( _dataType == "int64_t" )
    {
        int64_t data = (int64_t)_data;
        H5WriteStepAttribInt64(dataFile, _name.c_str(), &data, 1);
    }
    else if ( _dataType == "float" )
    {
        float data = (float)_data;
        H5WriteStepAttribFloat32(dataFile, _name.c_str(), &data, 1);
    }
    else if ( _dataType == "double" )
    {
        double data = (double)_data;
        H5WriteStepAttribFloat64(dataFile, _name.c_str(), &data, 1);
    }
}



// https://support.hdfgroup.org/HDF5/doc1.8/RM/RM_H5T.html
inline int HDF5SimIO::writeGridData()
{
    hid_t dataFile = H5Fcreate(outputFileName.c_str(), H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT); 
    dataFile = H5Fopen(outputFileName.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);

    for (int i=0; i<vars.size(); i++)
    {
        if ( vars[i].varType == grid_cellCentered )
        {
            hid_t  dataset, datatype;
            herr_t status;

            std::string _varname  = vars[i].name;

            int _numDims  = numDims;
            hsize_t *dims = new hsize_t[_numDims];
            for (int j=0; j<_numDims; j++)
                dims[j]=gridDims[j];

            datatype = getHDF5Datatype( vars[i].dataType );
            status   = H5Tset_order(datatype, endian==little?H5T_ORDER_LE:H5T_ORDER_BE);

            hid_t dataspace = H5Screate_simple(numDims, dims, NULL);
            dataset = H5Dcreate1(dataFile, _varname.c_str(), datatype, dataspace, H5P_DEFAULT);
            status  = H5Dwrite(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, vars[i].data);

            //
            // Clean up
            delete []dims;

            H5Sclose(dataspace);
            H5Tclose(datatype);
            H5Dclose(dataset);
        }   
    }

    H5Fclose(dataFile);
    return 0;
}


//https://accserv.lepp.cornell.edu/svn/packages/h5hut/examples/H5Part/H5PartTest.cc
// Viz: molecule plot in VisIt
inline int HDF5SimIO::writePointData(int ts)
{
    MPI_Init(NULL, NULL);

    int myRank, numProcs;
    MPI_Comm_rank(MPI_COMM_WORLD,&myRank);
    MPI_Comm_size(MPI_COMM_WORLD,&numProcs);
    

    h5_file_t *dataFile;
    dataFile = H5OpenFile(outputFileName.c_str(), H5_O_RDWR, MPI_COMM_WORLD);
    if (!dataFile)
        return -1;

    // Set timestep
    H5SetStep(dataFile, ts);  

    for (int i=0; i<timestepAttributes.size(); i++)
    {
        if ( timestepAttributes[i].dataType == "float" )
            H5WriteStepAttribInt32(dataFile, (timestepAttributes[i].name).c_str(), (int32_t *)timestepAttributes[i].data, timestepAttributes[i].numElements);
        else if ( timestepAttributes[i].dataType == "double" )
             H5WriteStepAttribInt64(dataFile, (vars[i].name).c_str(), (int64_t *)timestepAttributes[i].data, timestepAttributes[i].numElements);
        else if ( timestepAttributes[i].dataType == "int32_t" )
             H5WriteStepAttribFloat32(dataFile, (vars[i].name).c_str(), (float *)timestepAttributes[i].data, timestepAttributes[i].numElements);
        else if ( timestepAttributes[i].dataType == "int64_t" )
             H5WriteStepAttribFloat64(dataFile, (vars[i].name).c_str(), (double *)timestepAttributes[i].data, timestepAttributes[i].numElements);
        else
            return -1;
    }
    
    for (int i=0; i<vars.size(); i++)
    {
        H5PartSetNumParticles(dataFile, vars[i].numElements);

        if ( vars[i].dataType == "float" )
            H5PartWriteDataFloat32(dataFile, (vars[i].name).c_str(), (float *)vars[i].data);
        else if ( vars[i].dataType == "double" )
            H5PartWriteDataFloat64(dataFile, (vars[i].name).c_str(), (double *)vars[i].data);
        else if ( vars[i].dataType == "int32_t" )
            H5PartWriteDataInt32(dataFile, (vars[i].name).c_str(), (int32_t *)vars[i].data);
        else if ( vars[i].dataType == "int64_t" )
            H5PartWriteDataInt64(dataFile, (vars[i].name).c_str(), (int64_t *)vars[i].data);
        else
            return -1;
    }

    H5CloseFile(dataFile);

    MPI_Finalize();
    
    return 0;
}



inline int HDF5SimIO::writePointData()
{
    hid_t dataFile = H5Fcreate(outputFileName.c_str(), H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT); 
    dataFile = H5Fopen(outputFileName.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
    hid_t step = H5Gcreate(dataFile, "Step#0", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    hsize_t *dims = new hsize_t[1];
    dims[0]=vars[0].numElements;
    hid_t dataspace = H5Screate_simple(1, dims, NULL);
    hid_t dxpl = H5Pcreate(H5P_DATASET_XFER);

    for (int i=0; i<vars.size(); i++)
    {
        if ( vars[i].varType == point )
        {
            hid_t  dataset, datatype;
            herr_t status;

            std::string _varname = vars[i].name;

            datatype = getHDF5Datatype( vars[i].dataType );
            status   = H5Tset_order(datatype, endian==little?H5T_ORDER_LE:H5T_ORDER_BE);

            

            dataset = H5Dcreate1(step, _varname.c_str(), datatype, dataspace, H5P_DEFAULT);
            status  = H5Dwrite(dataset, datatype, H5S_ALL, datatype, dxpl, vars[i].data);

            
            H5Tclose(datatype);
            H5Dclose(dataset);
        }   
    }

    if (dims != NULL)
        delete []dims;

    H5Sclose(dataspace);
    H5Fclose(step);
    H5Fclose(dataFile);
    return 0;
}


} // end Flecsi_Sim_IO namespace

#endif