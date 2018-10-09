// Shape.cpp
// Implementations for Shape functions
//
// by Haoran Xiao, Mar 26, 2018
// based on Raster.cpp
//
// modified by Leland Pierce, June&July, 2018
//--------------------------------------------

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <sqlite3.h>
#include <exception>
#include <utility>

#include "H5Cpp.h"
//#include "File.hpp"
//#include "Vector.hpp"
//#include "Shape.hpp"
//#include "Exceptions.hpp"
//#include "attributes.hpp"


#include "geostar.hpp"

#include "spatialite_private.h"


//extern "C" {
//#include "string.h"
//#include "geoscipy_c.h"
//}

//extern "C" {
//sqlite3_vfs *sqlite3_HDFCPPvfs(void);
//#include "spatialite.h"
//#include "spatialite_private.h"
//}



#define DEBUG2

namespace GeoStar {

  //-----------------------------------------------------------------------
  // opening an existing Shape
  // non-existence will create a default Shape
  Shape::Shape(Vector *vec, const std::string &name) {
    if(vec->datasetExists(name)) {
#ifdef DEBUG2
      std::cout<<"shape constructor-opening: "<<std::endl;
#endif
      open(vec,name);
        //std::shared_ptr<SQLite::Database> dbPtrVal(db);
        //dbPtr = dbPtrVal;
    } else {
      // create a new Vector group:
#ifdef DEBUG2
      std::cout<<"shape constructor-creating: using default descriptor and srid"<<std::endl;
#endif
      std::string descriptor = "default shape description";
      int srid=1; // pixel/line coords
      int dims=2;
      create(vec,name,descriptor,srid,dims);
        //std::shared_ptr<SQLite::Database> dbPtrVal(db);
        //dbPtr = dbPtrVal;
    }//endif
    
  }// end-Shape-constructor
  


  //-----------------------------------------------------------------------
  // creates a Shape
  // if Shape exists, will cause exception
  Shape::Shape(Vector *vec, const std::string &name, const std::string &descriptor) {
    if (vec->datasetExists(name)) {
      db = NULL;
      throw_ShapeExistsError(name);
    } else {
      // create a new Shape
      int srid=1; // pixel/line coords
      int dims=2;
      create(vec,name,descriptor,srid,dims);
        //std::shared_ptr<SQLite::Database> dbPtrVal(db);
        //dbPtr = dbPtrVal;
    }//endif
  }// end-Shape-constructor


  //-----------------------------------------------------------------------
  // creates a Shape
  // if Shape exists, will cause exception
  Shape::Shape(Vector *vec, const std::string &name, 
               const std::string &descriptor, const int srid, const int dims) {
    if (vec->datasetExists(name)) {
      throw_ShapeExistsError(name);
    } else {
      // create a new Shape
      create(vec,name,descriptor,srid,dims);
        //std::shared_ptr<SQLite::Database> dbPtrVal(db);
        //dbPtr = dbPtrVal;
    }//endif
    
  }// end-Shape-constructor






  //-----------------------------------------------------------------------
  void Shape::open(Vector *vec, const std::string &name) {
#ifdef DEBUG2
    std::cout<<"Shape::open: start. name="<<name<<std::endl;
#endif

    shapename = "/"+vec->get_name()+"/"+name;
    shapeobj = new H5::DataSet(vec->openDataset(name));

#ifdef DEBUG2
      std::cout << "SHAPE NAME: " << shapename << "\n";
    std::cout<<"Shape::open: 1"<<std::endl;
#endif

    // check if its a valid Shape:
    if(read_object_type().find("geostar::shape")==std::string::npos) {
      delete shapeobj;
      throw_ShapeOpenError(name);
    }//endif
#ifdef DEBUG2
    std::cout<<"Shape::open: 2"<<std::endl;
#endif

    // open the ifile with a sqlite database in it
    db = vec->getGeoStarFile()->open_sqldatabase_ptr(shapename);

#ifdef DEBUG2
    std::cout<<"Shape::open: 3"<<std::endl;
#endif

    cache = spatialite_alloc_connection();
    spatialite_init_ex (db->getHandle(), cache, 0);

#ifdef DEBUG2
    std::cout<<"Shape::open: 4"<<std::endl;
#endif

    shapetype = "geostar::shape";

    this->srid = getSRID();
    this->dims = getDims();

#ifdef DEBUG2
    std::cout<<"Shape::open: done"<<std::endl;
#endif
      
      
      
      
      // NEW ... CLOSE THE Db HANDLE, SO OTHERS CAN USE IT LATER ...
      //delete db;

  }//end-func: open








  //-----------------------------------------------------------------------
  void Shape::create(Vector *vec, const std::string &name, 
                     const std::string &descriptor, const int srid,
                     const int dims) {

#ifdef DEBUG2
    std::cout<<"Shape::create: start. name="<<name<<std::endl;
#endif
    shapename = "/"+vec->get_name()+"/"+name;
    if(vec->getGeoStarFile()->datasetExists(shapename)){
      throw_ShapeExistsError(shapename);
    }//endif

#ifdef DEBUG2
    std::cout<<"Shape::create: 1. shapename="<<shapename<<std::endl;
#endif

    // these 2 must come before creating the sqldatabase
    //sqlite3_vfs_register(sqlite3_HDFCPPvfs(), 1);

#ifdef DEBUG2
    std::cout<<"Shape::create: 2. "<<std::endl;
#endif



#ifdef DEBUG2
    std::cout<<"Shape::create: 3. "<<std::endl;
#endif

    // create the ifile with a sqlite database in it
    db = vec->getGeoStarFile()->create_sqldatabase_ptr(shapename);

#ifdef DEBUG2
    std::cout<<"Shape::create: 4. "<<std::endl;
#endif

    cache = spatialite_alloc_connection();
    spatialite_init_ex (db->getHandle(), cache, 0);

    // make the sqlite database a spatialite database
    int ret = db->exec("SELECT InitSpatialMetadata(1)");

#ifdef DEBUG2
    std::cout<<"Shape::create: 5. "<<std::endl;
#endif

#ifdef DEBUG22

    std::cout<<"Shape::create: 5: ret="<<ret<<std::endl;
    std::cout<<"Shape::create: 5: db->getLastInsertRowid()="<<db->getLastInsertRowid()<<std::endl;
    std::cout<<"Shape::create: 5: db->getErrorMsg()="<<db->getErrorMsg()<<std::endl;


    std::cout<<"Shape::create: 5. sqlite_master defn:"<<std::endl;
    {//start-scope
      SQLite::Statement   query(*db, "PRAGMA table_info(sqlite_master)");

    while (query.executeStep())
    {
        const char* value1   = query.getColumn(0);
        const char* value2   = query.getColumn(1);
        const char* value3   = query.getColumn(2);
        const char* value4   = query.getColumn(3);
        std::cout << value1<<", "<<value2<<", "<<value3<<", "<<value4<<std::endl;
    }
    }//end-scope

    std::cout<<"Shape::create: 5. sqlite_master contents:"<<std::endl;
    {//start-scope
      //SQLite::Statement   query(*db, "SELECT name FROM sqlite_master WHERE type='table'");
      SQLite::Statement   query(*db, "SELECT * FROM sqlite_master");
    //SQLite::Statement   query(*db, "SELECT * FROM spatial_ref_sys");
      //SQLite::Statement   query(*db, "PRAGMA table_info(spatialite_history)");

    while (query.executeStep())
    {
        const char* value1   = query.getColumn(0);
        const char* value2   = query.getColumn(1);
        const char* value3   = query.getColumn(2);
        const char* value4   = query.getColumn(4);
        std::cout << value1<<", "<<value2<<", "<<value3<<", "<<value4<<std::endl;
    }
    }//end-scope

    std::cout<<"Shape::create: 5: db->getLastInsertRowid()="<<db->getLastInsertRowid()<<std::endl;
    std::cout<<"Shape::create: 5: db->getErrorMsg()="<<db->getErrorMsg()<<std::endl;

#endif

    addSpatialReferenceSystems();

#ifdef DEBUG2
    std::cout<<"Shape::create: 6. "<<std::endl;
#endif

    createShapeTable(srid, dims);
    this->srid = srid;
    this->dims = dims;

    shapeobj = new H5::DataSet(vec->openDataset(shapename));

    shapetype = "geostar::shape";
    GeoStar::append_object_type((H5::H5Object *)shapeobj, shapetype);


#ifdef DEBUG2
    std::cout<<"Shape::create: DONE. "<<std::endl;
#endif

    

  }//end-func: create




  //-----------------------------------------------------------------------
  // a shape is a single table named shape containing things each of which
  // can be any type of 2d or 3d  geometry (point, line, polygon).
  // dims must be either 2 or 3.
  //-----------------------------------------------------------------------
  void Shape::createShapeTable(const int srid, const int dims)
  {
    int ret=0;
    // 1. create the table
    ret = db->exec("CREATE TABLE IF NOT EXISTS shape (PK INTEGER NOT NULL PRIMARY KEY)");
    
    if(dims==2) {
    // 2. add a 2D geometry column
    ret = db->exec("SELECT AddGeometryColumn('shape', 'geometry',"+std::to_string(srid)+
                   ", 'GEOMETRYCOLLECTION', 2)");
    } else if(dims==3) {
    ret = db->exec("SELECT AddGeometryColumn('shape', 'geometry',"+std::to_string(srid)+
                   ", 'GEOMETRYCOLLECTION', 3)");
    } else {
      throw std::runtime_error("Shape::createShapeTable:: invalid value for dims");
    }//endif

    // 3. make geometry column have r*tree spatial index
    ret = db->exec("SELECT CreateSpatialIndex('shape', 'geometry')");

  }//end-func: createShapeTable


  //-----------------2D------------------------------------------------------
  void Shape::addPoint(const double x, const double y)
  {
    if(dims!=2) throw std::runtime_error("Shape::addPoint: wrong dimensionality");

    SpatiaLite::GeometryCollection *gc = new SpatiaLite::GeometryCollection();

    gc->setSRID(getSRID());
    gc->setType(GAIA_GEOMETRYCOLLECTION);
    gc->addPoint(x,y);

    SpatiaLite::Blob *blob = SpatiaLite::Blob::toSpatiaLiteBlobWkb(gc);

    SQLite::Statement   query(*db, "INSERT INTO shape (geometry) VALUES (?)");
    query.bind(1, (void *)blob->get(), blob->getSize());
    query.exec();

    delete blob;
    delete gc;
  }//end-func: addPoint

  //-----------------3D------------------------------------------------------
  void Shape::addPoint(const double x, const double y, const double z)
  {
    if(dims!=3) throw std::runtime_error("Shape::addPoint: wrong dimensionality");

    SpatiaLite::GeometryCollection *gc = new SpatiaLite::GeometryCollection();

    gc->setSRID(getSRID());
    gc->setType(GAIA_GEOMETRYCOLLECTION);
    gc->addPoint(x,y,z);

    SpatiaLite::Blob *blob = SpatiaLite::Blob::toSpatiaLiteBlobWkb(gc);

    SQLite::Statement   query(*db, "INSERT INTO shape (geometry) VALUES (?)");
    query.bind(1, (void *)blob->get(), blob->getSize());
    query.exec();

    delete blob;
    delete gc;
  }//end-func: addPoint


  //-----------------2D------------------------------------------------------
   template <typename T>
    void Shape::addPoint(const SimpleFeatures::Point<T> pt)
   {
    if(dims!=2) throw std::runtime_error("Shape::addPoint: wrong dimensionality");

     SpatiaLite::GeometryCollection *gc = new SpatiaLite::GeometryCollection();
     gc->setSRID(getSRID());
     gc->setType(GAIA_GEOMETRYCOLLECTION);
     gc->addPoint(pt);
  
     SpatiaLite::Blob *blob = SpatiaLite::Blob::toSpatiaLiteBlobWkb(gc);

     SQLite::Statement   query(*db, "INSERT INTO shape (geometry) VALUES (?)");
     query.bind(1, (void *)blob->get(), blob->getSize());
     query.exec();

     delete blob;
     delete gc;
   }//end-func: addPoint

  template void Shape::addPoint<double>(const SimpleFeatures::Point<double> pt);
  template void Shape::addPoint<int>(const SimpleFeatures::Point<int> pt);

  //-----------------3D------------------------------------------------------
   template <typename T>
    void Shape::addPoint(const SimpleFeatures::Point3D<T> pt)
   {
    if(dims!=3) throw std::runtime_error("Shape::addPoint: wrong dimensionality");

     SpatiaLite::GeometryCollection *gc = new SpatiaLite::GeometryCollection();
     gc->setSRID(getSRID());
     gc->setType(GAIA_GEOMETRYCOLLECTION);
     gc->addPoint(pt);
  
     SpatiaLite::Blob *blob = SpatiaLite::Blob::toSpatiaLiteBlobWkb(gc);

     SQLite::Statement   query(*db, "INSERT INTO shape (geometry) VALUES (?)");
     query.bind(1, (void *)blob->get(), blob->getSize());
     query.exec();

     delete blob;
     delete gc;
   }//end-func: addPoint

  template void Shape::addPoint<double>(const SimpleFeatures::Point3D<double> pt);
  template void Shape::addPoint<int>(const SimpleFeatures::Point3D<int> pt);

  //-----------------2D------------------------------------------------------
   template <typename T>
    void Shape::addLine(const SimpleFeatures::Line<T> line)
   {
    if(dims!=2) throw std::runtime_error("Shape::addPoint: wrong dimensionality");

     SpatiaLite::GeometryCollection *gc = new SpatiaLite::GeometryCollection();
     gc->setSRID(getSRID());
     gc->setType(GAIA_GEOMETRYCOLLECTION);
     gc->addLine(line);
  
     SpatiaLite::Blob *blob = SpatiaLite::Blob::toSpatiaLiteBlobWkb(gc);

     SQLite::Statement   query(*db, "INSERT INTO shape (geometry) VALUES (?)");
     query.bind(1, (void *)blob->get(), blob->getSize());
     query.exec();

     delete blob;
     delete gc;
   }//end-func: addLine

  template void Shape::addLine<double>(const SimpleFeatures::Line<double> line);
  template void Shape::addLine<int>(const SimpleFeatures::Line<int> line);

  //-----------------3D------------------------------------------------------
   template <typename T>
    void Shape::addLine(const SimpleFeatures::Line3D<T> line)
   {
    if(dims!=3) throw std::runtime_error("Shape::addPoint: wrong dimensionality");

     SpatiaLite::GeometryCollection *gc = new SpatiaLite::GeometryCollection();
     gc->setSRID(getSRID());
     gc->setType(GAIA_GEOMETRYCOLLECTION);
     gc->addLine(line);
  
     SpatiaLite::Blob *blob = SpatiaLite::Blob::toSpatiaLiteBlobWkb(gc);

     SQLite::Statement   query(*db, "INSERT INTO shape (geometry) VALUES (?)");
     query.bind(1, (void *)blob->get(), blob->getSize());
     query.exec();

     delete blob;
     delete gc;
   }//end-func: addLine

  template void Shape::addLine<double>(const SimpleFeatures::Line3D<double> line);
  template void Shape::addLine<int>(const SimpleFeatures::Line3D<int> line);

  //-----------------2D------------------------------------------------------
   template <typename T>
    void Shape::addPolygon(const SimpleFeatures::Polygon<T> poly)
   {
    if(dims!=2) throw std::runtime_error("Shape::addPoint: wrong dimensionality");

     SpatiaLite::GeometryCollection *gc = new SpatiaLite::GeometryCollection();
     gc->setSRID(getSRID());
     gc->setType(GAIA_GEOMETRYCOLLECTION);
#ifdef DEBUG2
     std::cout<<"addPolygon: a"<<std::endl;
#endif
     //gc->addPoint(88.9,33.1);
     gc->addPolygon(poly);
 #ifdef DEBUG2
     std::cout<<"addPolygon: b"<<std::endl;
#endif
 
     SpatiaLite::Blob *blob = SpatiaLite::Blob::toSpatiaLiteBlobWkb(gc);

     SQLite::Statement   query(*db, "INSERT INTO shape (geometry) VALUES (?)");
     query.bind(1, (void *)blob->get(), blob->getSize());
     query.exec();

#ifdef DEBUG2
     std::cout<<"addPolygon: c"<<std::endl;
#endif

     delete blob;
#ifdef DEBUG2
     std::cout<<"addPolygon: d"<<std::endl;
#endif
     delete gc;
#ifdef DEBUG2
     std::cout<<"addPolygon: e"<<std::endl;
#endif

   
   }//end-func: addLine

    template void Shape::addPolygon<double>(const SimpleFeatures::Polygon<double> poly);
    template void Shape::addPolygon<int>(const SimpleFeatures::Polygon<int> poly);
    template void Shape::addPolygon<float>(const SimpleFeatures::Polygon<float> poly);
    template void Shape::addPolygon<uint8_t>(const SimpleFeatures::Polygon<uint8_t> poly);


  //-----------------2D------------------------------------------------------
   template <typename T>
    void Shape::addPolygon(const SimpleFeatures::Polygon3D<T> poly)
   {
    if(dims!=3) throw std::runtime_error("Shape::addPoint: wrong dimensionality");

     SpatiaLite::GeometryCollection *gc = new SpatiaLite::GeometryCollection();
     gc->setSRID(getSRID());
     gc->setType(GAIA_GEOMETRYCOLLECTION);
#ifdef DEBUG2
     std::cout<<"addPolygon: a"<<std::endl;
#endif
     gc->addPolygon(poly);
 #ifdef DEBUG2
     std::cout<<"addPolygon: b"<<std::endl;
#endif
 
     SpatiaLite::Blob *blob = SpatiaLite::Blob::toSpatiaLiteBlobWkb(gc);

     SQLite::Statement   query(*db, "INSERT INTO shape (geometry) VALUES (?)");
     query.bind(1, (void *)blob->get(), blob->getSize());
     query.exec();

#ifdef DEBUG2
     std::cout<<"addPolygon: c"<<std::endl;
#endif

     delete blob;
#ifdef DEBUG2
     std::cout<<"addPolygon: d"<<std::endl;
#endif
     delete gc;
#ifdef DEBUG2
     std::cout<<"addPolygon: e"<<std::endl;
#endif

   
   }//end-func: addLine

  template void Shape::addPolygon<double>(const SimpleFeatures::Polygon3D<double> poly);
  template void Shape::addPolygon<int>(const SimpleFeatures::Polygon3D<int> poly);






















  //------------------just debug---------------------------------------------
  void Shape::getGeometries()
  {
    // the function context scopes this "Statement"
    // this gets every row....
    SQLite::Statement   query(*db, "SELECT geometry from shape");
    
    // get contents of rows 1-at-a-time:
    while(query.executeStep()){
      SQLite::Column col = query.getColumn(0);
      std::cout<<"Shape::getGeometries: "<<SpatiaLite::GeometryCollection(col).toWKTString()<<std::endl;
    }//endwhile


  }//end-func: getGeometries

  std::vector<std::pair<int, std::string>> Shape::getWithin(std::string geoText) {
    std::string srid_str = std::to_string(srid);
    SQLite::Statement query (*db, "select PK, CastToSingle(geometry) from shape WHERE Within(CastToSingle(geometry), GeomFromText('" + geoText + "', " + srid_str + "))");
	
    std::vector<std::pair<int, std::string>> res;
    while (query.executeStep()) {
	int PK = query.getColumn(0);
        SQLite::Column col = query.getColumn(1);
	std::pair<int, std::string> p = std::make_pair(PK, SpatiaLite::GeometryCollection(col).toWKTString());
	res.push_back(p);
	//std::cout << "getWithin: "<<p.first <<" "<<p.second<<std::endl;
    }
    //return output
    return res;
}

    std::vector<std::pair<int, double>> Shape::getLength() {
      SQLite::Statement query (*db, "select PK, GLength(CastToSingle(geometry)) from shape WHERE GeometryType(CastToSingle(geometry)) = 'LINESTRING'");

      std::vector<std::pair<int, double>> res;
      while (query.executeStep()) {
	int PK = query.getColumn(0);
	SQLite::Column col = query.getColumn(1);
        std::pair<int, double> p = std::make_pair(PK, col);
	res.push_back(p);
    }
    return res;
}

    std::vector<std::pair<int, double>> Shape::getPerimeter() {
      SQLite::Statement query (*db, "select PK, Perimeter(CastToSingle(geometry)) from shape WHERE GeometryType(CastToSingle(geometry)) = 'POLYGON'");

      std::vector<std::pair<int, double>> res;
      while (query.executeStep()) {
	int PK = query.getColumn(0);
	SQLite::Column col = query.getColumn(1);
	std::pair<int, double> p = std::make_pair(PK, col);
	res.push_back(p);
        //std::cout << "getPerimeter: "<< col <<std::endl;
    }
      return res;
}

    std::vector<std::pair<int, double>> Shape::getArea() {
      SQLite::Statement query (*db, "select PK, Area(CastToSingle(geometry)) from shape WHERE GeometryType(CastToSingle(geometry)) = 'POLYGON'");

      std::vector<std::pair<int, double>> res;
      while (query.executeStep()) {
	int PK = query.getColumn(0);
	SQLite::Column col = query.getColumn(1);
        //std::cout << "getArea: "<< col <<std::endl;
	std::pair<int, double> p = std::make_pair(PK, col);
	res.push_back(p);
    }
      return res;
}

    std::vector<std::pair<int, std::string>> Shape::getBoundary() {
      SQLite::Statement query (*db, "select PK, Boundary(CastToSingle(geometry)) from shape");

      std::vector<std::pair<int, std::string>> res;
      while (query.executeStep()) {
	int PK = query.getColumn(0);
	SQLite::Column col = query.getColumn(1);
	std::pair<int, std::string> p = std::make_pair(PK, SpatiaLite::GeometryCollection(col).toWKTString());
	res.push_back(p);
        //std::cout << "getBoundary: "<< col <<std::endl;
    }
      return res;
}

    std::vector<std::pair<int, std::string>> Shape::getEnvelope() {
      SQLite::Statement query (*db, "select PK, Envelope(CastToSingle(geometry)) from shape");

      std::vector<std::pair<int, std::string>> res;
      while (query.executeStep()) {
	int PK = query.getColumn(0);
	SQLite::Column col = query.getColumn(1);
	std::pair<int, std::string> p = std::make_pair(PK, SpatiaLite::GeometryCollection(col).toWKTString());
	res.push_back(p);
        //std::cout << "getEnvelope: "<< col <<std::endl;
    }
      return res;
}

    std::vector<std::pair<int, std::string>> Shape::getExpand() {
      SQLite::Statement query (*db, "select PK, ST_Expand(CastToSingle(geometry), 1) from shape");

      std::vector<std::pair<int, std::string>> res;
      while (query.executeStep()) {
	int PK = query.getColumn(0);
	SQLite::Column col = query.getColumn(1);
	std::pair<int, std::string> p = std::make_pair(PK, SpatiaLite::GeometryCollection(col).toWKTString());
	res.push_back(p);
        //std::cout << "getExpand: "<< col <<std::endl;
    }
    return res;
}
    std::vector<std::pair<int, std::string>> Shape::getCentroid() {
      SQLite::Statement query (*db, "select PK, Centroid(CastToSingle(geometry)) from shape");

      std::vector<std::pair<int, std::string>> res;
      while (query.executeStep()) {
	int PK = query.getColumn(0);
	SQLite::Column col = query.getColumn(1);
	std::pair<int, std::string> p = std::make_pair(PK, SpatiaLite::GeometryCollection(col).toWKTString());
	res.push_back(p);
        //std::cout << "getCentroid: "<< col <<std::endl;
    }
    return res;
}

    std::vector<std::pair<int, std::string>> Shape::getSimplify() {
      SQLite::Statement query (*db, "select PK, Simplify(CastToSingle(geometry), 0.5) from shape where GeometryType(CastToSingle(geometry)) = 'LINESTRING' OR GeometryType(CastToSingle(geometry)) = 'RING'");

      std::vector<std::pair<int, std::string>> res;
      while (query.executeStep()) {
	int PK = query.getColumn(0);
	SQLite::Column col = query.getColumn(1);
	std::pair<int, std::string> p = std::make_pair(PK, SpatiaLite::GeometryCollection(col).toWKTString());
	res.push_back(p);
        //std::cout << "getSimplify: "<< col <<std::endl;
    }
    return res;
}

  //-----------------------------------------------------------------------
  void Shape::addSpatialReferenceSystems()
  {
    // adds spatialrefsystems for 
    // 1. non-georef'd data,
    // 2. planetary bodies other than the Earth, from the IAU2000 database.


    
    // 1. Put in initial non-geographical coord system (allow simple file/pixel addresses)
    //int srid = 1;
    //std::string auth_name = "CUSTOM";
    //int auth_srid = -1;
    //std::string ref_sys_name = "Custom";
    //std::string proj4text = "";
    //std::string srs_wkt = "";
    db->exec("INSERT INTO spatial_ref_sys " 
             "(srid,      auth_name, auth_srid, ref_sys_name, proj4text, srtext) VALUES(" 
             " \"1\",    \"CUSTOM\",    \"-1\",   \"Custom\",      \"\",   \"\")");


    // 2. Open IAU200 file with the WKT defns in it
    //    If gdal is installed in /blah/gdal, then this file is in /blah/gdal/data
    const char *pszFilename;
    // this file is in the gdal "data" directory, wherever that is installed....
    pszFilename = CPLFindFile( "gdal", "IAU2000.wkt" );
    if (pszFilename == NULL) {
      std::cerr << "can't find gdal file IAU2000.wkt";
      return;  // not bad enough to die.
    }//endif
    
    std::ifstream infile(pszFilename);
    if (!infile) {
      std::cerr << "unable to open file pszFilename";
      return; // not bad enough to die.
    }//endif


    // 3. Declarations for converting from/to wkt and proj4
    OGRSpatialReference asr;
    
    OGRErr eErr;
    char *wkt1;
    char *proj41;
    std::string pszLine;
    

    // 4. loop over each wkt definition in the file:
    while ( getline(infile,pszLine) ) {
      
      // 4.1 ignore comments
      if (pszLine.compare(0,1,"#") == 0)    // if first char is "#" just a comment, ignore and get next line
        continue;
      
      // 4.2 ignore irrelevant files
      std::size_t comma = pszLine.find(",");
      if (comma == std::string::npos)      // if no "," in this line, ignore and get next line
        continue;
      
      // 4.3 parse the first integer: it's their SRID
      int thecode = std::stoi (pszLine.substr(0,comma));
      
      // 4.4 the rest of the line is the WKT:
      std::string remainingLine = pszLine.substr(comma+1);
      char *pszLineChars = (char *)(remainingLine.c_str());
      
      // 4.5 import the WKT into an OGR spatialRef, ignore if error
      eErr = asr.importFromWkt( &pszLineChars );
      if (eErr != OGRERR_NONE) {
        std::cerr << "importFromWkt ERROR!  eErr=" << eErr << "  for line: \n";
        std::cerr << pszLine << "\n";
        continue;
      }
      
      // 4.6 since their SRIDs overlap with ones used for Earth,
      //     add a large number to put them beyond the Earth SRIDs:
      int srid=900000+thecode;

      // 4.7 convert to a well-formed WKT string:
      eErr = asr.exportToWkt(&wkt1);
      if (eErr != OGRERR_NONE) {
        std::cerr << "exportToWkt ERROR!  eErr=" << eErr << "  for line: \n";
        std::cerr << pszLine << "\n";
        continue;
      }
      std::string wktString(wkt1);


      // 4.8 convert to a PROJ4 string:
      eErr = exportIAUToProj4(asr, &proj41);
      if (eErr != OGRERR_NONE) {
        std::cerr << "exportToProj4 ERROR!  eErr=" << eErr << "  for line: \n";
        std::cerr << pszLine << "\n";
        exit(-1); //continue;
      }
      std::string proj41String(proj41);

      // 4.9 prepare for writing to database:
      std::string name;
      if (asr.IsProjected())
        name = std::string(asr.GetAttrValue("PROJCS",0));
      else
        name = std::string(asr.GetAttrValue("GEOGCS",0));
      
      // 4.10 Replace any " characters with '  (double quotes with single quotes)
      const std::string doubleQuote = "\"";
      const std::string singleQuote = "'";
      std::string::size_type doubleQuoteSize = doubleQuote.size();
      std::string::size_type n = 0;
      while ((n = wktString.find(doubleQuote,n)) != std::string::npos)
        wktString.replace(n,doubleQuoteSize,singleQuote);
      
      // 4.11 write to the database:
      // NOTE:
      //std::string auth_name = "iau2000";
      //int auth_srid = thecode;
      //std::string ref_sys_name = name;
      //std::string proj4text = proj41String;
      //std::string srs_wkt = wktString;
      if (wktString.length() == 0) wktString = "Undefined";
      
      std::string executeCmd("INSERT INTO spatial_ref_sys ");
      executeCmd.append("(srid, auth_name, auth_srid, ref_sys_name, proj4text, srtext) VALUES(");
      executeCmd.append(std::to_string(srid)+", \"iau2000\", ");
      executeCmd.append(std::to_string(thecode)+", \"" + name + "\", \"");
      executeCmd.append(proj41String+"\", \""+ wktString + "\")");
      
      try {
        db->exec(executeCmd);
      } catch (...) {
        // ignore errors.
      }//end-catch

    }// end while: read next record in .wkt file

    infile.close();
    
  }//end-func: addSpatialReferenceSystems
  





















  
#ifdef NOTYET
  //-----------------------------------------------------------------------
  std::vector<std::string> Shape::get_column_as_string(const std::string & colname) {
    std::string sql_str = "SELECT " + colname + " FROM multipolygon";
    sqlite3_stmt * stmt;
    std::vector<std::string> output;
    const char * sql = sql_str.c_str();
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
      throw_ShapeReadError("Shape::get_column_as_string: prepare_v2");
    }
    // sqlite3_column_text outputs const unsigned char *
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
      output.emplace_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt,0)));
    }
    if (rc != SQLITE_DONE) {
      throw_ShapeReadError("Shape::get_column_as_string: not done");
    }
    sqlite3_finalize(stmt);
    return output;		
  }
  
  std::vector<std::string> Shape::get_geoms_as_WKT() {
    sqlite3_stmt * stmt;
    std::vector<std::string> output;
    const char * sql = "SELECT AsText(geom) FROM multipolygon"; // converst to WKT string
    // const char * sql = "SELECT AsBinary(geom) FROM multipolygon"; // converts to WKB string, does not display when converted to text
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
      throw_ShapeReadError("Shape::get_geoms_as_WKT: prepare_v2");
    }
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
      std::string s(reinterpret_cast<const char *>(sqlite3_column_text(stmt,0)));
      output.push_back(s);
    }
    if (rc != SQLITE_DONE) {
      throw_ShapeReadError("Shape::get_geoms_as_WKT: not done");
    }
    sqlite3_finalize(stmt);
    return output;
  }
#endif //NOTYET





  //------------------------------------------------------------------------------
  //------------------------------------------------------------------------------
  //------------------------------------------------------------------------------
  //------------------------------------------------------------------------------
  //----------reworked gdal code so it works for IAU200 wkt data------------------
  //------------------------------------------------------------------------------
  //------------------------------------------------------------------------------
  //------------------------------------------------------------------------------
  //------------------------------------------------------------------------------
  //------------------------------------------------------------------------------
  //------------------------------------------------------------------------------

  //------------------------------------------------------------------------------
typedef struct
{
    const char* pszWKTName;
    const char* pszValueInMeter;
    const char* pszProjName;
} LinearUnitsStruct;

static const LinearUnitsStruct asLinearUnits [] =
{
{ SRS_UL_METER, "1.0", "m" },
{ SRS_UL_METER, "1.0", "meter" }, // alias of former
{ SRS_UL_METER, "1.0", "metre" }, // alias of former
{ "metre", "1.0", "m" }, // alias of former
 // Leave as 'kilometre' instead of SRS_UL_KILOMETER due to historical usage.
{ "kilometre", SRS_UL_KILOMETER_CONV, "km" },
{ SRS_UL_KILOMETER, SRS_UL_KILOMETER_CONV, "km" }, // alias of former
{ SRS_UL_DECIMETER, SRS_UL_DECIMETER_CONV, "dm" },
{ SRS_UL_CENTIMETER, SRS_UL_CENTIMETER_CONV, "cm" },
{ SRS_UL_MILLIMETER, SRS_UL_MILLIMETER_CONV, "mm" },

 // Leave as 'Foot (International)' or SRS_UL_FOOT instead of SRS_UL_INTL_FOOT
// due to historical usage.
{ SRS_UL_FOOT, SRS_UL_FOOT_CONV, "ft" },
{ SRS_UL_INTL_FOOT, SRS_UL_INTL_FOOT_CONV, "ft" }, // alias of former
{ SRS_UL_US_FOOT, SRS_UL_US_FOOT_CONV, "us-ft" },
{ SRS_UL_INDIAN_FOOT, SRS_UL_INDIAN_FOOT_CONV, "ind-ft" },

{ SRS_UL_INTL_NAUT_MILE, SRS_UL_INTL_NAUT_MILE_CONV, "kmi" },
{ SRS_UL_NAUTICAL_MILE, SRS_UL_NAUTICAL_MILE_CONV, "kmi" }, // alias of former

{ SRS_UL_INTL_STAT_MILE, SRS_UL_INTL_STAT_MILE_CONV, "mi" },
{ "Mile", SRS_UL_INTL_STAT_MILE_CONV, "mi" }, // alias of former
{ "IMILE", SRS_UL_INTL_STAT_MILE_CONV, "mi" }, // alias of former
{ SRS_UL_US_STAT_MILE, SRS_UL_US_STAT_MILE_CONV, "us-mi"},

{ SRS_UL_INTL_LINK, SRS_UL_INTL_LINK_CONV, "link" },
{ SRS_UL_LINK, SRS_UL_LINK_CONV, "link" }, // alias of former

{ SRS_UL_INTL_YARD, SRS_UL_INTL_YARD_CONV, "yd" },
{ "IYARD", SRS_UL_INTL_YARD_CONV, "yd" }, // alias of former
{ SRS_UL_US_YARD, SRS_UL_US_YARD_CONV, "us-yd" },
{ SRS_UL_INDIAN_YARD, SRS_UL_INDIAN_YARD_CONV, "ind-yd" },

{ SRS_UL_INTL_INCH, SRS_UL_INTL_INCH_CONV, "in" },
{ SRS_UL_US_INCH, SRS_UL_US_INCH_CONV, "us-in" },

{ SRS_UL_INTL_FATHOM, SRS_UL_INTL_FATHOM_CONV, "fath" },

{ SRS_UL_INTL_CHAIN, SRS_UL_INTL_CHAIN_CONV, "ch" },
{ SRS_UL_US_CHAIN, SRS_UL_US_CHAIN_CONV, "us-ch" },
{ SRS_UL_INDIAN_CHAIN, SRS_UL_INDIAN_CHAIN_CONV, "ind-ch" },

// { SRS_UL_ROD, SRS_UL_ROD_CONV, "????" }
};

  //------------------------------------------------------------------------------


/************************************************************************/
/*                        GetLinearFromLinearConvOrName()               */
/************************************************************************/

static const LinearUnitsStruct *LEPGetLinearFromLinearConvOrName(
    double dfLinearConv,
    const char *pszLinearUnits )

{
    for( size_t i = 0; i < CPL_ARRAYSIZE(asLinearUnits); i++ )
    {
        if( (pszLinearUnits != nullptr &&
             EQUAL(pszLinearUnits, asLinearUnits[i].pszWKTName)) ||
            fabs(dfLinearConv -
                 CPLAtof(asLinearUnits[i].pszValueInMeter)) < 0.00000001 )
        {
            return &(asLinearUnits[i]);
        }
    }
    return nullptr;
}

  //------------------------------------------------------------------------------

#define SAFE_PROJ4_STRCAT(szNewStr) do { \
    if( CPLStrlcat(szProj4, szNewStr, sizeof(szProj4)) >= sizeof(szProj4) ) { \
        CPLError(CE_Failure, CPLE_AppDefined, \
                 "String overflow when formatting proj.4 string"); \
        *ppszProj4 = CPLStrdup(""); \
        return OGRERR_FAILURE; \
    } } while( false );

  //------------------------------------------------------------------------------
  int Shape::exportIAUToProj4( const OGRSpatialReference &asr, char ** ppszProj4 ) const
{
    if( asr.GetRoot() == nullptr )
    {
        *ppszProj4 = CPLStrdup("");
        CPLError( CE_Failure, CPLE_NotSupported,
                  "No translation for an empty SRS to PROJ.4 format is known.");
        return OGRERR_UNSUPPORTED_SRS;
    }

/* -------------------------------------------------------------------- */
/*      Do we have a PROJ.4 override definition?                        */
/* -------------------------------------------------------------------- */
    const char *pszPredefProj4 = asr.GetExtension( asr.GetRoot()->GetValue(),
                                               "PROJ4", nullptr );
    if( pszPredefProj4 != nullptr )
    {
        *ppszProj4 = CPLStrdup( pszPredefProj4 );
        return OGRERR_NONE;
    }

/* -------------------------------------------------------------------- */
/*      Get the prime meridian info.                                    */
/* -------------------------------------------------------------------- */
    const OGR_SRSNode *poPRIMEM = asr.GetAttrNode( "PRIMEM" );
    double l_dfFromGreenwich = 0.0;

    if( poPRIMEM != nullptr && poPRIMEM->GetChildCount() >= 2
        && CPLAtof(poPRIMEM->GetChild(1)->GetValue()) != 0.0 )
    {
        l_dfFromGreenwich = CPLAtof(poPRIMEM->GetChild(1)->GetValue());
    }

/* ==================================================================== */
/*      Handle the projection definition.                               */
/* ==================================================================== */

    const char *pszProjection = asr.GetAttrValue("PROJECTION");

    char szProj4[512] = {};

    // TODO(schwehr): Cleanup CPLsnprintf calls to do less pointer arithmetic.
    if( !asr.IsProjected() && asr.IsGeographic() )
    {
        CPLsnprintf(szProj4 + strlen(szProj4),
                    sizeof(szProj4) - strlen(szProj4),
                    "+proj=longlat ");
    }
    else if( asr.IsGeocentric() )
    {
        CPLsnprintf(szProj4 + strlen(szProj4)
                    , sizeof(szProj4) - strlen(szProj4),
                    "+proj=geocent ");
    }

    else if( !asr.IsProjected() && !asr.IsGeographic() )
    {
        // LOCAL_CS, or incompletely initialized coordinate systems.
        *ppszProj4 = CPLStrdup("");
#ifdef DEBUG2
        std::cout<< "Shape::exportIAUToProj4: ERROR: not-projected and not-geographic"<<std::endl;
#endif
        return OGRERR_NONE;
    }
    else if( strstr(pszProjection, SRS_PT_CYLINDRICAL_EQUAL_AREA) )
    {
        CPLsnprintf(
            szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
            "+proj=cea +lon_0=%.16g +lat_ts=%.16g +x_0=%.16g +y_0=%.16g ",
            asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
            asr.GetNormProjParm(SRS_PP_STANDARD_PARALLEL_1, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }

    else if( strstr(pszProjection, SRS_PT_BONNE) )
    {
        CPLsnprintf(
            szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
            "+proj=bonne +lon_0=%.16g +lat_1=%.16g +x_0=%.16g +y_0=%.16g ",
            asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
            asr.GetNormProjParm(SRS_PP_STANDARD_PARALLEL_1, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }

    else if( strstr(pszProjection, SRS_PT_CASSINI_SOLDNER) )
    {
        CPLsnprintf(
            szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
            "+proj=cass +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g ",
            asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
            asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }

    else if( strstr(pszProjection, SRS_PT_NEW_ZEALAND_MAP_GRID) )
    {
        CPLsnprintf(
            szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
            "+proj=nzmg +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g ",
            asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
            asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }

    else if( strstr(pszProjection, SRS_PT_TRANSVERSE_MERCATOR) ||
             strstr(pszProjection, SRS_PT_TRANSVERSE_MERCATOR_MI_21) ||
             strstr(pszProjection, SRS_PT_TRANSVERSE_MERCATOR_MI_22) ||
             strstr(pszProjection, SRS_PT_TRANSVERSE_MERCATOR_MI_23) ||
             strstr(pszProjection, SRS_PT_TRANSVERSE_MERCATOR_MI_24) ||
             strstr(pszProjection, SRS_PT_TRANSVERSE_MERCATOR_MI_25) )
    {
        int bNorth = FALSE;
        const int nZone = asr.GetUTMZone( &bNorth );

        const char* pszUseETMERC = CPLGetConfigOption("OSR_USE_ETMERC", nullptr);
        if( pszUseETMERC && CPLTestBool(pszUseETMERC) )
        {
            CPLsnprintf( szProj4 + strlen(szProj4),
                         sizeof(szProj4) - strlen(szProj4),
                         "+proj=etmerc +lat_0=%.16g +lon_0=%.16g +k=%.16g "
                         "+x_0=%.16g +y_0=%.16g ",
                         asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
                         asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
                         asr.GetNormProjParm(SRS_PP_SCALE_FACTOR, 1.0),
                         asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
                         asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
        }
        else if( pszUseETMERC && !CPLTestBool(pszUseETMERC) )
        {
            CPLsnprintf( szProj4 + strlen(szProj4),
                         sizeof(szProj4) - strlen(szProj4),
                         "+proj=tmerc +lat_0=%.16g +lon_0=%.16g +k=%.16g "
                         "+x_0=%.16g +y_0=%.16g ",
                         asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
                         asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
                         asr.GetNormProjParm(SRS_PP_SCALE_FACTOR, 1.0),
                         asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
                         asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
        }
        else if( nZone != 0 )
        {
            if( bNorth )
                CPLsnprintf( szProj4 + strlen(szProj4),
                             sizeof(szProj4) - strlen(szProj4),
                             "+proj=utm +zone=%d ",
                             nZone );
            else
                CPLsnprintf( szProj4 + strlen(szProj4),
                             sizeof(szProj4) - strlen(szProj4),
                             "+proj=utm +zone=%d +south ",
                             nZone );
        }
        else
            CPLsnprintf( szProj4 + strlen(szProj4),
                         sizeof(szProj4) - strlen(szProj4),
                         "+proj=tmerc +lat_0=%.16g +lon_0=%.16g +k=%.16g "
                         "+x_0=%.16g +y_0=%.16g ",
                         asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
                         asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
                         asr.GetNormProjParm(SRS_PP_SCALE_FACTOR, 1.0),
                         asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
                         asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_TRANSVERSE_MERCATOR_SOUTH_ORIENTED) )
    {
        CPLsnprintf( szProj4 + strlen(szProj4),
                     sizeof(szProj4) - strlen(szProj4),
                     "+proj=tmerc +lat_0=%.16g +lon_0=%.16g +k=%.16g "
                     "+x_0=%.16g +y_0=%.16g +axis=wsu ",
                     asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
                     asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
                     asr.GetNormProjParm(SRS_PP_SCALE_FACTOR, 1.0),
                     asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
                     asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_MERCATOR_1SP) )
    {
        if( asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0) == 0.0 )
            CPLsnprintf( szProj4 + strlen(szProj4),
                         sizeof(szProj4) - strlen(szProj4),
                         "+proj=merc +lon_0=%.16g +k=%.16g +x_0=%.16g "
                         "+y_0=%.16g ",
                         asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
                         asr.GetNormProjParm(SRS_PP_SCALE_FACTOR, 1.0),
                         asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
                         asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
        else if( asr.GetNormProjParm(SRS_PP_SCALE_FACTOR, 1.0) == 1.0 )
            CPLsnprintf( szProj4 + strlen(szProj4),
                         sizeof(szProj4) - strlen(szProj4),
                         "+proj=merc +lon_0=%.16g +lat_ts=%.16g +x_0=%.16g "
                         "+y_0=%.16g ",
                         asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
                         asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
                         asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
                         asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
        else
        {
            CPLError(CE_Failure, CPLE_NotSupported,
                     "Mercator_1SP with scale != 1.0 and "
                     "latitude of origin != 0, not supported by PROJ.4.");
            *ppszProj4 = CPLStrdup("");
            return OGRERR_UNSUPPORTED_SRS;
        }
    }
    else if( strstr(pszProjection, SRS_PT_MERCATOR_2SP) )
    {
        if( asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0) == 0.0 )
        {
            CPLsnprintf(
                szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
                "+proj=merc +lon_0=%.16g +lat_ts=%.16g +x_0=%.16g +y_0=%.16g ",
                asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
                asr.GetNormProjParm(SRS_PP_STANDARD_PARALLEL_1, 0.0),
                asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
                asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
        }
        else
        {
            CPLError(CE_Failure, CPLE_NotSupported,
                     "Mercator_2SP with "
                     "latitude of origin != 0, not supported by PROJ.4.");
            *ppszProj4 = CPLStrdup("");
            return OGRERR_UNSUPPORTED_SRS;
        }
    }
    else if( strstr(pszProjection, "Mercator") )
    {
      CPLsnprintf(
                  szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
                  "+proj=merc +lon_0=%.16g +lat_ts=%.16g +x_0=%.16g +y_0=%.16g ",
                  asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
                  asr.GetNormProjParm(SRS_PP_STANDARD_PARALLEL_1, 0.0),
                  asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
                  asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_MERCATOR_AUXILIARY_SPHERE) )
    {
       // This is EPSG:3875 Pseudo Mercator. No point in trying to parse the
       // rest of the parameters, since we know pretty much everything at this
       // stage.
       CPLsnprintf( szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
                    "+proj=merc +a=%.16g +b=%.16g +lat_ts=%.16g "
                    "+lon_0=%.16g +x_0=%.16g +y_0=%.16g +k=%.16g +units=m "
                    "+nadgrids=@null +wktext  +no_defs",
                    asr.GetSemiMajor(), asr.GetSemiMajor(),
                    asr.GetNormProjParm(SRS_PP_STANDARD_PARALLEL_1, 0.0),
                    asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
                    asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
                    asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0),
                    asr.GetNormProjParm(SRS_PP_SCALE_FACTOR, 1.0) );
       *ppszProj4 = CPLStrdup( szProj4 );

       return OGRERR_NONE;
    }
    else if( strstr(pszProjection, SRS_PT_OBLIQUE_STEREOGRAPHIC) )
    {
        CPLsnprintf(
           szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
           "+proj=sterea +lat_0=%.16g +lon_0=%.16g +k=%.16g "
           "+x_0=%.16g +y_0=%.16g ",
           asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
           asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
           asr.GetNormProjParm(SRS_PP_SCALE_FACTOR, 1.0),
           asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
           asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_STEREOGRAPHIC) )
    {
        CPLsnprintf(
            szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
            "+proj=stere +lat_0=%.16g +lon_0=%.16g +k=%.16g "
            "+x_0=%.16g +y_0=%.16g ",
            asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
            asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
            asr.GetNormProjParm(SRS_PP_SCALE_FACTOR, 1.0),
            asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_POLAR_STEREOGRAPHIC) )
    {
        if( asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0) >= 0.0 )
            CPLsnprintf(
                szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
                "+proj=stere +lat_0=90 +lat_ts=%.16g +lon_0=%.16g "
                "+k=%.16g +x_0=%.16g +y_0=%.16g ",
                asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 90.0),
                asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
                asr.GetNormProjParm(SRS_PP_SCALE_FACTOR, 1.0),
                asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
                asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
        else
            CPLsnprintf(
                szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
                "+proj=stere +lat_0=-90 +lat_ts=%.16g +lon_0=%.16g "
                "+k=%.16g +x_0=%.16g +y_0=%.16g ",
                asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, -90.0),
                asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
                asr.GetNormProjParm(SRS_PP_SCALE_FACTOR, 1.0),
                asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
                asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_EQUIRECTANGULAR) ||
             strstr(pszProjection, "Equidistant_Cylindrical")  )
    {
        CPLsnprintf(
            szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
            "+proj=eqc +lat_ts=%.16g +lat_0=%.16g +lon_0=%.16g "
            "+x_0=%.16g +y_0=%.16g ",
            asr.GetNormProjParm(SRS_PP_STANDARD_PARALLEL_1, 0.0),
            asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
            asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_GAUSSSCHREIBERTMERCATOR) )
    {
        CPLsnprintf(
            szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
            "+proj=gstmerc +lat_0=%.16g +lon_0=%.16g "
            "+k_0=%.16g +x_0=%.16g +y_0=%.16g ",
            asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, -21.116666667),
            asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 55.53333333309),
            asr.GetNormProjParm(SRS_PP_SCALE_FACTOR, 1.0),
            asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 160000.000),
            asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 50000.000) );
    }
    else if( strstr(pszProjection, SRS_PT_GNOMONIC) )
    {
        CPLsnprintf(
            szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
            "+proj=gnom +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g ",
            asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
            asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_ORTHOGRAPHIC) )
    {
        CPLsnprintf(
            szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
            "+proj=ortho +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g ",
            asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
            asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_LAMBERT_AZIMUTHAL_EQUAL_AREA) )
    {
        CPLsnprintf(
            szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
            "+proj=laea +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g ",
            asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
            asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_AZIMUTHAL_EQUIDISTANT) )
    {
        CPLsnprintf(
            szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
            "+proj=aeqd +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g ",
            asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
            asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_EQUIDISTANT_CONIC) )
    {
        CPLsnprintf(
            szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
            "+proj=eqdc +lat_0=%.16g +lon_0=%.16g +lat_1=%.16g +lat_2=%.16g "
            "+x_0=%.16g +y_0=%.16g ",
            asr.GetNormProjParm(SRS_PP_LATITUDE_OF_CENTER, 0.0),
            asr.GetNormProjParm(SRS_PP_LONGITUDE_OF_CENTER, 0.0),
            asr.GetNormProjParm(SRS_PP_STANDARD_PARALLEL_1, 0.0),
            asr.GetNormProjParm(SRS_PP_STANDARD_PARALLEL_2, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_MILLER_CYLINDRICAL) )
    {
        CPLsnprintf(
            szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
            "+proj=mill +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g +R_A ",
            asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
            asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_MOLLWEIDE) )
    {
        CPLsnprintf(
            szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
            "+proj=moll +lon_0=%.16g +x_0=%.16g +y_0=%.16g ",
            asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_ECKERT_I) )
    {
        CPLsnprintf(
            szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
            "+proj=eck1 +lon_0=%.16g +x_0=%.16g +y_0=%.16g ",
            asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_ECKERT_II) )
    {
        CPLsnprintf(
            szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
            "+proj=eck2 +lon_0=%.16g +x_0=%.16g +y_0=%.16g ",
            asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_ECKERT_III) )
    {
        CPLsnprintf(
            szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
            "+proj=eck3 +lon_0=%.16g +x_0=%.16g +y_0=%.16g ",
            asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_ECKERT_IV) )
    {
        CPLsnprintf(
            szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
            "+proj=eck4 +lon_0=%.16g +x_0=%.16g +y_0=%.16g ",
            asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_ECKERT_V) )
    {
        CPLsnprintf(
            szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
            "+proj=eck5 +lon_0=%.16g +x_0=%.16g +y_0=%.16g ",
            asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_ECKERT_VI) )
    {
        CPLsnprintf(
            szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
            "+proj=eck6 +lon_0=%.16g +x_0=%.16g +y_0=%.16g ",
            asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
            asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_POLYCONIC) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=poly +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
             asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_ALBERS_CONIC_EQUAL_AREA) ||
             strstr(pszProjection, "Albers")    )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=aea +lat_1=%.16g +lat_2=%.16g +lat_0=%.16g +lon_0=%.16g "
             "+x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_STANDARD_PARALLEL_1, 0.0),
             asr.GetNormProjParm(SRS_PP_STANDARD_PARALLEL_2, 0.0),
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
             asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_ROBINSON) )
    {
        // Workaround a bug in proj.4 :
        // https://github.com/OSGeo/proj.4/commit/
        //                              bc7453d1a75aab05bdff2c51ed78c908e3efa3cd
        const double dfLon0 = asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0);
        const double dfX0 = asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0);
        const double dfY0 = asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0);
        if( CPLIsNan(dfLon0) || CPLIsNan(dfX0) || CPLIsNan(dfY0) )
        {
            return OGRERR_FAILURE;
        }
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=robin +lon_0=%.16g +x_0=%.16g +y_0=%.16g ",
             dfLon0,
             dfX0,
             dfY0 );
    }
    else if( strstr(pszProjection, SRS_PT_VANDERGRINTEN) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=vandg +lon_0=%.16g +x_0=%.16g +y_0=%.16g +R_A ",
             asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_SINUSOIDAL) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=sinu +lon_0=%.16g +x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_LONGITUDE_OF_CENTER, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_GALL_STEREOGRAPHIC) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=gall +lon_0=%.16g +x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_GOODE_HOMOLOSINE) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=goode +lon_0=%.16g +x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_IGH) )
    {
        CPLsnprintf( szProj4 + strlen(szProj4),
                     sizeof(szProj4) - strlen(szProj4),
                     "+proj=igh " );
    }
    else if( strstr(pszProjection, SRS_PT_GEOSTATIONARY_SATELLITE) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=geos +lon_0=%.16g +h=%.16g +x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
             asr.GetNormProjParm(SRS_PP_SATELLITE_HEIGHT, 35785831.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_LAMBERT_CONFORMAL_CONIC_2SP) ||
             strstr(pszProjection, SRS_PT_LAMBERT_CONFORMAL_CONIC_2SP_BELGIUM) ||
             strstr(pszProjection, "Lambert_Conformal_Conic")                      )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=lcc +lat_1=%.16g +lat_2=%.16g +lat_0=%.16g +lon_0=%.16g "
             "+x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_STANDARD_PARALLEL_1, 0.0),
             asr.GetNormProjParm(SRS_PP_STANDARD_PARALLEL_2, 0.0),
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
             asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_LAMBERT_CONFORMAL_CONIC_1SP) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=lcc +lat_1=%.16g +lat_0=%.16g +lon_0=%.16g "
             "+k_0=%.16g +x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
             asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
             asr.GetNormProjParm(SRS_PP_SCALE_FACTOR, 1.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_HOTINE_OBLIQUE_MERCATOR) )
    {
        // Special case for swiss oblique mercator: see bug 423.
        if( fabs(asr.GetNormProjParm(SRS_PP_AZIMUTH, 0.0) - 90.0) < 0.0001
            && fabs(asr.GetNormProjParm(SRS_PP_RECTIFIED_GRID_ANGLE, 0.0)
                    - 90.0) < 0.0001 )
        {
            CPLsnprintf(
                 szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
                 "+proj=somerc +lat_0=%.16g +lon_0=%.16g "
                 "+k_0=%.16g +x_0=%.16g +y_0=%.16g ",
                 asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
                 asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
                 asr.GetNormProjParm(SRS_PP_SCALE_FACTOR, 1.0),
                 asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
                 asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
        }
        else
        {
            CPLsnprintf(
                 szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
                 "+proj=omerc +lat_0=%.16g +lonc=%.16g +alpha=%.16g "
                 "+k=%.16g +x_0=%.16g +y_0=%.16g +no_uoff ",
                 asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
                 asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
                 asr.GetNormProjParm(SRS_PP_AZIMUTH, 0.0),
                 asr.GetNormProjParm(SRS_PP_SCALE_FACTOR, 1.0),
                 asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
                 asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );

            // RSO variant: http://trac.osgeo.org/proj/ticket/62
            // Note that gamma is only supported by PROJ 4.8.0 and later.
            if( asr.GetNormProjParm(SRS_PP_RECTIFIED_GRID_ANGLE, 1000.0) != 1000.0 )
            {
                CPLsnprintf(
                     szProj4 + strlen(szProj4),
                     sizeof(szProj4) - strlen(szProj4),
                     "+gamma=%.16g ",
                     asr.GetNormProjParm(SRS_PP_RECTIFIED_GRID_ANGLE, 1000.0));
            }
        }
    }
    else if( strstr(pszProjection,
                   SRS_PT_HOTINE_OBLIQUE_MERCATOR_AZIMUTH_CENTER) )
    {
        // Special case for swiss oblique mercator: see bug 423.
        if( fabs(asr.GetNormProjParm(SRS_PP_AZIMUTH, 0.0) - 90.0) < 0.0001 &&
            fabs(asr.GetNormProjParm(SRS_PP_RECTIFIED_GRID_ANGLE, 0.0)
                 - 90.0) < 0.0001 )
        {
            CPLsnprintf(
                 szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
                 "+proj=somerc +lat_0=%.16g +lon_0=%.16g "
                 "+k_0=%.16g +x_0=%.16g +y_0=%.16g ",
                 asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
                 asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
                 asr.GetNormProjParm(SRS_PP_SCALE_FACTOR, 1.0),
                 asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
                 asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
        }
        else
        {
            CPLsnprintf(
                 szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
                 "+proj=omerc +lat_0=%.16g +lonc=%.16g +alpha=%.16g "
                 "+k=%.16g +x_0=%.16g +y_0=%.16g ",
                 asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
                 asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
                 asr.GetNormProjParm(SRS_PP_AZIMUTH, 0.0),
                 asr.GetNormProjParm(SRS_PP_SCALE_FACTOR, 1.0),
                 asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
                 asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );

            // RSO variant: http://trac.osgeo.org/proj/ticket/62
            // Note that gamma is only supported by PROJ 4.8.0 and later.
            if( asr.GetNormProjParm(SRS_PP_RECTIFIED_GRID_ANGLE, 1000.0) != 1000.0 )
            {
                CPLsnprintf(szProj4 + strlen(szProj4),
                            sizeof(szProj4) - strlen(szProj4),
                            "+gamma=%.16g ",
                            asr.GetNormProjParm(SRS_PP_RECTIFIED_GRID_ANGLE,
                                            1000.0));
            }
        }
    }
    else if( strstr(pszProjection,
                   SRS_PT_HOTINE_OBLIQUE_MERCATOR_TWO_POINT_NATURAL_ORIGIN) )
    {
        // Not really clear which of Point_1/1st_Point convention is the
        // "normalized" one, so accept both.
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=omerc +lat_0=%.16g "
             "+lon_1=%.16g +lat_1=%.16g +lon_2=%.16g +lat_2=%.16g "
             "+k=%.16g +x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
             asr.GetNormProjParm(SRS_PP_LONGITUDE_OF_POINT_1,
                             asr.GetNormProjParm(SRS_PP_LONGITUDE_OF_1ST_POINT,
                                             0.0)),
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_POINT_1,
                             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_1ST_POINT,
                                             0.0)),
             asr.GetNormProjParm(SRS_PP_LONGITUDE_OF_POINT_2,
                             asr.GetNormProjParm(SRS_PP_LONGITUDE_OF_2ND_POINT,
                                             0.0)),
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_POINT_2,
                             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_2ND_POINT,
                                             0.0)),
             asr.GetNormProjParm(SRS_PP_SCALE_FACTOR, 1.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_KROVAK) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=krovak +lat_0=%.16g +lon_0=%.16g +alpha=%.16g "
             "+k=%.16g +x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_CENTER, 0.0),
             asr.GetNormProjParm(SRS_PP_LONGITUDE_OF_CENTER, 0.0),
             asr.GetNormProjParm(SRS_PP_AZIMUTH, 0.0),
             asr.GetNormProjParm(SRS_PP_SCALE_FACTOR, 1.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_TWO_POINT_EQUIDISTANT) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=tpeqd +lat_1=%.16g +lon_1=%.16g "
             "+lat_2=%.16g +lon_2=%.16g "
             "+x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_1ST_POINT, 0.0),
             asr.GetNormProjParm(SRS_PP_LONGITUDE_OF_1ST_POINT, 0.0),
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_2ND_POINT, 0.0),
             asr.GetNormProjParm(SRS_PP_LONGITUDE_OF_2ND_POINT, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_IMW_POLYCONIC) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=iwm_p +lat_1=%.16g +lat_2=%.16g +lon_0=%.16g "
             "+x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_1ST_POINT, 0.0),
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_2ND_POINT, 0.0),
             asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_WAGNER_I) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=wag1 +x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_WAGNER_II) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=wag2 +x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_WAGNER_III) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=wag3 +lat_ts=%.16g +x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_WAGNER_IV) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=wag4 +x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_WAGNER_V) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=wag5 +x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_WAGNER_VI) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=wag6 +x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_WAGNER_VII) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=wag7 +x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_QSC) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=qsc +lat_0=%.16g +lon_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
             asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_SCH) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=sch +plat_0=%.16g +plon_0=%.16g +phdg_0=%.16g +h_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_PEG_POINT_LATITUDE, 0.0),
             asr.GetNormProjParm(SRS_PP_PEG_POINT_LONGITUDE, 0.0),
             asr.GetNormProjParm(SRS_PP_PEG_POINT_HEADING, 0.0),
             asr.GetNormProjParm(SRS_PP_PEG_POINT_HEIGHT, 0.0) );
    }
    // Note: This never really gets used currently.  See bug 423.
    else if( strstr(pszProjection, SRS_PT_SWISS_OBLIQUE_CYLINDRICAL) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=somerc +lat_0=%.16g +lon_0=%.16g "
             "+x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
             asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_AITOFF) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=aitoff +lat_0=%.16g +lon_0=%.16g "
             "+x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
             asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0) );
    }
    else if( strstr(pszProjection, SRS_PT_WINKEL_I) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=wink1 +lat_0=%.16g +lon_0=%.16g lat_ts=%.16g "
             "+x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
             asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
             asr.GetNormProjParm(SRS_PP_STANDARD_PARALLEL_1, 45.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0));
    }
    else if( strstr(pszProjection, SRS_PT_WINKEL_II) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=wink2 +lat_0=%.16g +lon_0=%.16g +lat_1=%.16g "
             "+x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
             asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
             asr.GetNormProjParm(SRS_PP_STANDARD_PARALLEL_1, 40.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0));
    }
    else if( strstr(pszProjection, SRS_PT_WINKEL_TRIPEL) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=wintri +lat_0=%.16g +lon_0=%.16g +lat_1=%.16g "
             "+x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
             asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
             asr.GetNormProjParm(SRS_PP_STANDARD_PARALLEL_1, 40.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0));
    }
    else if( strstr(pszProjection, SRS_PT_CRASTER_PARABOLIC) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=crast +lat_0=%.16g +lon_0=%.16g "
             "+x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
             asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0));
    }
    else if( strstr(pszProjection, SRS_PT_LOXIMUTHAL) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=loxim +lon_0=%.16g +lat_1=%.16g "
             "+x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 40.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0));
    }
    else if( strstr(pszProjection, SRS_PT_QUARTIC_AUTHALIC) )
    {
        CPLsnprintf(
             szProj4 + strlen(szProj4), sizeof(szProj4) - strlen(szProj4),
             "+proj=qua_aut +lat_0=%.16g +lon_0=%.16g "
             "+x_0=%.16g +y_0=%.16g ",
             asr.GetNormProjParm(SRS_PP_LATITUDE_OF_ORIGIN, 0.0),
             asr.GetNormProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_EASTING, 0.0),
             asr.GetNormProjParm(SRS_PP_FALSE_NORTHING, 0.0));
    }
    else
    {
        CPLError( CE_Failure, CPLE_NotSupported,
                  "No translation for %s to PROJ.4 format is known.",
                  pszProjection );
#ifdef DEBUG2
        std::cout<<"No translation for "<<pszProjection<<" to PROJ.4 format is known."<<std::endl;
#endif
        *ppszProj4 = CPLStrdup("");
        return OGRERR_UNSUPPORTED_SRS;
    }

/* -------------------------------------------------------------------- */
/*      Handle earth model.  For now we just always emit the user       */
/*      defined ellipsoid parameters.                                   */
/* -------------------------------------------------------------------- */
    const double dfSemiMajor = asr.GetSemiMajor();
    const double dfInvFlattening = asr.GetInvFlattening();
    const char *pszPROJ4Ellipse = nullptr;
    const char *pszDatum = asr.GetAttrValue("DATUM");

    if( std::abs(dfSemiMajor-6378249.145) < 0.01
        && std::abs(dfInvFlattening-293.465) < 0.0001 )
    {
        pszPROJ4Ellipse = "clrk80";     // Clark 1880
    }
    else if( std::abs(dfSemiMajor-6378245.0) < 0.01
             && std::abs(dfInvFlattening-298.3) < 0.0001 )
    {
        pszPROJ4Ellipse = "krass";      // Krassovsky
    }
    else if( std::abs(dfSemiMajor-6378388.0) < 0.01
             && std::abs(dfInvFlattening-297.0) < 0.0001 )
    {
        pszPROJ4Ellipse = "intl";       // International 1924
    }
    else if( std::abs(dfSemiMajor-6378160.0) < 0.01
             && std::abs(dfInvFlattening-298.25) < 0.0001 )
    {
        pszPROJ4Ellipse = "aust_SA";    // Australian
    }
    else if( std::abs(dfSemiMajor-6377397.155) < 0.01
             && std::abs(dfInvFlattening-299.1528128) < 0.0001 )
    {
        pszPROJ4Ellipse = "bessel";     // Bessel 1841
    }
    else if( std::abs(dfSemiMajor-6377483.865) < 0.01
             && std::abs(dfInvFlattening-299.1528128) < 0.0001 )
    {
        pszPROJ4Ellipse = "bess_nam";   // Bessel 1841 (Namibia / Schwarzeck)
    }
    else if( std::abs(dfSemiMajor-6378160.0) < 0.01
             && std::abs(dfInvFlattening-298.247167427) < 0.0001 )
    {
        pszPROJ4Ellipse = "GRS67";      // GRS 1967
    }
    else if( std::abs(dfSemiMajor-6378137) < 0.01
             && std::abs(dfInvFlattening-298.257222101) < 0.000001 )
    {
        pszPROJ4Ellipse = "GRS80";      // GRS 1980
    }
    else if( std::abs(dfSemiMajor-6378206.4) < 0.01
             && std::abs(dfInvFlattening-294.9786982) < 0.0001 )
    {
        pszPROJ4Ellipse = "clrk66";     // Clarke 1866
    }
    else if( std::abs(dfSemiMajor-6377340.189) < 0.01
             && std::abs(dfInvFlattening-299.3249646) < 0.0001 )
    {
        pszPROJ4Ellipse = "mod_airy";   // Modified Airy
    }
    else if( std::abs(dfSemiMajor-6377563.396) < 0.01
             && std::abs(dfInvFlattening-299.3249646) < 0.0001 )
    {
        pszPROJ4Ellipse = "airy";       // Airy
    }
    else if( std::abs(dfSemiMajor-6378200) < 0.01
             && std::abs(dfInvFlattening-298.3) < 0.0001 )
    {
        pszPROJ4Ellipse = "helmert";    // Helmert 1906
    }
    else if( std::abs(dfSemiMajor-6378155) < 0.01
             && std::abs(dfInvFlattening-298.3) < 0.0001 )
    {
        pszPROJ4Ellipse = "fschr60m";   // Modified Fischer 1960
    }
    else if( std::abs(dfSemiMajor-6377298.556) < 0.01
             && std::abs(dfInvFlattening-300.8017) < 0.0001 )
    {
        pszPROJ4Ellipse = "evrstSS";    // Everest (Sabah & Sarawak)
    }
    else if( std::abs(dfSemiMajor-6378165.0) < 0.01
             && std::abs(dfInvFlattening-298.3) < 0.0001 )
    {
        pszPROJ4Ellipse = "WGS60";
    }
    else if( std::abs(dfSemiMajor-6378145.0) < 0.01
             && std::abs(dfInvFlattening-298.25) < 0.0001 )
    {
        pszPROJ4Ellipse = "WGS66";
    }
    else if( std::abs(dfSemiMajor-6378135.0) < 0.01
             && std::abs(dfInvFlattening-298.26) < 0.0001 )
    {
        pszPROJ4Ellipse = "WGS72";
    }
    else if( std::abs(dfSemiMajor-6378137.0) < 0.01
             && std::abs(dfInvFlattening-298.257223563) < 0.000001 )
    {
        pszPROJ4Ellipse = "WGS84";
    }
    else if( pszDatum != nullptr && EQUAL(pszDatum, "North_American_Datum_1927") )
    {
        // pszPROJ4Ellipse = "clrk66:+datum=nad27";  // NAD 27
        pszPROJ4Ellipse = "clrk66";
    }
    else if( pszDatum != nullptr && EQUAL(pszDatum, "North_American_Datum_1983") )
    {
        // pszPROJ4Ellipse = "GRS80:+datum=nad83";  // NAD 83
        pszPROJ4Ellipse = "GRS80";
    }

    char szEllipseDef[128] = {};

    if( pszPROJ4Ellipse == nullptr )
        CPLsnprintf( szEllipseDef, sizeof(szEllipseDef), "+a=%.16g +b=%.16g ",
                     asr.GetSemiMajor(), asr.GetSemiMinor() );
    else
        CPLsnprintf( szEllipseDef, sizeof(szEllipseDef), "+ellps=%s ",
                 pszPROJ4Ellipse );

/* -------------------------------------------------------------------- */
/*      Translate the datum.                                            */
/* -------------------------------------------------------------------- */
    const char *pszPROJ4Datum = nullptr;
    const OGR_SRSNode *poTOWGS84 = asr.GetAttrNode( "TOWGS84" );
    int nEPSGDatum = -1;
    int nEPSGGeogCS = -1;
    const char *pszProj4Grids = asr.GetExtension( "DATUM", "PROJ4_GRIDS" );

    const char *pszAuthority = asr.GetAuthorityName( "DATUM" );

    if( pszAuthority != nullptr && EQUAL(pszAuthority, "EPSG") )
        nEPSGDatum = atoi(asr.GetAuthorityCode( "DATUM" ));

    const char *pszGeogCSAuthority = asr.GetAuthorityName( "GEOGCS" );

    if( pszGeogCSAuthority != nullptr && EQUAL(pszGeogCSAuthority, "EPSG") )
        nEPSGGeogCS = atoi(asr.GetAuthorityCode( "GEOGCS" ));

    if( pszDatum == nullptr )
    {
        // Nothing.
    }
    else if( EQUAL(pszDatum, SRS_DN_NAD27) || nEPSGDatum == 6267 )
    {
        pszPROJ4Datum = "NAD27";
    }
    else if( EQUAL(pszDatum, SRS_DN_NAD83) || nEPSGDatum == 6269 )
    {
        pszPROJ4Datum = "NAD83";
    }
    else if( EQUAL(pszDatum, SRS_DN_WGS84) || nEPSGDatum == 6326 )
    {
        pszPROJ4Datum = "WGS84";
    }

    if( pszProj4Grids != nullptr )
    {
        SAFE_PROJ4_STRCAT( szEllipseDef );
        szEllipseDef[0] = '\0';
        SAFE_PROJ4_STRCAT( "+nadgrids=" );
        SAFE_PROJ4_STRCAT( pszProj4Grids );
        SAFE_PROJ4_STRCAT( " " );
        pszPROJ4Datum = nullptr;
    }

    if( pszPROJ4Datum == nullptr
        || CPLTestBool(CPLGetConfigOption("OVERRIDE_PROJ_DATUM_WITH_TOWGS84",
                                          "YES")) )
    {
        if( poTOWGS84 != nullptr )
        {
            if( poTOWGS84->GetChildCount() >= 3
                && (poTOWGS84->GetChildCount() < 7
                    || (EQUAL(poTOWGS84->GetChild(3)->GetValue(), "")
                        && EQUAL(poTOWGS84->GetChild(4)->GetValue(), "")
                        && EQUAL(poTOWGS84->GetChild(5)->GetValue(), "")
                        && EQUAL(poTOWGS84->GetChild(6)->GetValue(), ""))) )
            {
                SAFE_PROJ4_STRCAT( szEllipseDef );
                szEllipseDef[0] = '\0';
                SAFE_PROJ4_STRCAT( "+towgs84=");
                for( int iChild = 0; iChild < 3; iChild++ )
                {
                    if( iChild > 0 ) SAFE_PROJ4_STRCAT( "," );
                    SAFE_PROJ4_STRCAT(poTOWGS84->GetChild(iChild)->GetValue());
                }
                SAFE_PROJ4_STRCAT( " " );
                pszPROJ4Datum = nullptr;
            }
            else if( poTOWGS84->GetChildCount() >= 7)
            {
                SAFE_PROJ4_STRCAT( szEllipseDef );
                szEllipseDef[0] = '\0';
                SAFE_PROJ4_STRCAT( "+towgs84=");
                for( int iChild = 0; iChild < 7; iChild++ )
                {
                    if( iChild > 0 ) SAFE_PROJ4_STRCAT( "," );
                    SAFE_PROJ4_STRCAT(poTOWGS84->GetChild(iChild)->GetValue());
                }
                SAFE_PROJ4_STRCAT( " " );
                pszPROJ4Datum = nullptr;
            }
        }

    }

    if( pszPROJ4Datum != nullptr )
    {
        SAFE_PROJ4_STRCAT( "+datum=" );
        SAFE_PROJ4_STRCAT( pszPROJ4Datum );
        SAFE_PROJ4_STRCAT( " " );
    }
    else
    {
        // The ellipsedef may already have been appended and will now
        // be empty, otherwise append now.
        SAFE_PROJ4_STRCAT( szEllipseDef );
        // szEllipseDef[0] = '\0';
    }

/* -------------------------------------------------------------------- */
/*      Is there prime meridian info to apply?                          */
/* -------------------------------------------------------------------- */
    if( poPRIMEM != nullptr && poPRIMEM->GetChildCount() >= 2
        && CPLAtof(poPRIMEM->GetChild(1)->GetValue()) != 0.0 )
    {
        
        char szPMValue[128] = {};
            CPLsnprintf( szPMValue, sizeof(szPMValue),
                         "%.16g", l_dfFromGreenwich );

        SAFE_PROJ4_STRCAT( "+pm=" );
        SAFE_PROJ4_STRCAT( szPMValue );
        SAFE_PROJ4_STRCAT( " " );
    }

/* -------------------------------------------------------------------- */
/*      Handle linear units.                                            */
/* -------------------------------------------------------------------- */
    const char *pszPROJ4Units=nullptr;
    char *pszLinearUnits = nullptr;
    double dfLinearConv = asr.GetLinearUnits( &pszLinearUnits );

    if( strstr(szProj4, "longlat") != nullptr )
    {
        pszPROJ4Units = nullptr;
    }
    else
    {
        const LinearUnitsStruct* psLinearUnits =
            LEPGetLinearFromLinearConvOrName( dfLinearConv, pszLinearUnits );
        if( psLinearUnits != nullptr )
            pszPROJ4Units = psLinearUnits->pszProjName;
        else
            pszPROJ4Units = nullptr;

        if( pszPROJ4Units == nullptr )
        {
            char szLinearConv[128] = {};
            CPLsnprintf( szLinearConv, sizeof(szLinearConv),
                         "%.16g", dfLinearConv );
            SAFE_PROJ4_STRCAT( "+to_meter=" );
            SAFE_PROJ4_STRCAT( szLinearConv );
            SAFE_PROJ4_STRCAT( " " );
        }
    }

    if( pszPROJ4Units != nullptr )
    {
        SAFE_PROJ4_STRCAT( "+units=");
        SAFE_PROJ4_STRCAT( pszPROJ4Units );
        SAFE_PROJ4_STRCAT( " " );
    }

/* -------------------------------------------------------------------- */
/*   If we have vertical datum grids, attach them to the proj.4 string. */
/* -------------------------------------------------------------------- */
    const char *pszProj4Geoids = asr.GetExtension( "VERT_DATUM", "PROJ4_GRIDS" );

    if( pszProj4Geoids != nullptr )
    {
        SAFE_PROJ4_STRCAT( "+geoidgrids=" );
        SAFE_PROJ4_STRCAT( pszProj4Geoids );
        SAFE_PROJ4_STRCAT( " " );
    }

/* -------------------------------------------------------------------- */
/*      Handle vertical units, but only if we have them.                */
/* -------------------------------------------------------------------- */
    const OGR_SRSNode *poVERT_CS = asr.GetRoot()->GetNode( "VERT_CS" );
    const OGR_SRSNode *poVUNITS = nullptr;

    if( poVERT_CS != nullptr )
        poVUNITS = poVERT_CS->GetNode( "UNIT" );

    if( poVUNITS != nullptr && poVUNITS->GetChildCount() >= 2 )
    {
        pszPROJ4Units = nullptr;

        dfLinearConv = CPLAtof( poVUNITS->GetChild(1)->GetValue() );

        const LinearUnitsStruct* psLinearUnits =
            LEPGetLinearFromLinearConvOrName(dfLinearConv,
                                          poVUNITS->GetChild(0)->GetValue());
        if( psLinearUnits != nullptr )
            pszPROJ4Units = psLinearUnits->pszProjName;
        else
            pszPROJ4Units = nullptr;

        if( pszPROJ4Units == nullptr )
        {
            char szLinearConv[128] = {};
            CPLsnprintf( szLinearConv, sizeof(szLinearConv),
                         "%.16g", dfLinearConv );
            SAFE_PROJ4_STRCAT( "+vto_meter=" );
            SAFE_PROJ4_STRCAT( szLinearConv );
            SAFE_PROJ4_STRCAT( " " );
        }
        else
        {
            SAFE_PROJ4_STRCAT( "+vunits=");
            SAFE_PROJ4_STRCAT( pszPROJ4Units );
            SAFE_PROJ4_STRCAT( " " );
        }
    }

/* -------------------------------------------------------------------- */
/*      Add the no_defs flag to ensure that no values from              */
/*      proj_def.dat are implicitly used with our definitions.          */
/* -------------------------------------------------------------------- */
    SAFE_PROJ4_STRCAT( "+no_defs " );

    *ppszProj4 = CPLStrdup( szProj4 );

    return OGRERR_NONE;
}//end-function: exportIAUToProj4



//----------------------------------------------------------------------
//----------------------------------------------------------------------
void Shape::updateSRID(const int srid)
{
  // update the table that keeps track of these things:
  db->exec("UPDATE geometry_columns SET srid = "+std::to_string(srid)+
                 " WHERE f_table_name = 'shape'");

  // update the actual shape table:
  db->exec("UPDATE shape SET geometry = SetSRID(geometry, "+
                 std::to_string(srid)+")");

  this->srid = srid;
  
}//end-function: updateSRID

//----------------------------------------------------------------------
//----------------------------------------------------------------------
int Shape::getSRID()
{
  // the function context scopes this "Statement"
  SQLite::Statement   query(*db, "SELECT srid from geometry_columns WHERE f_table_name = 'shape'");

  // just once:
  std::string srid("1"); // pixel/line is default....
  if(query.executeStep()){
    srid = (const char *)query.getColumn(0);

#ifdef DEBUG2
    std::cout<<"Shape::getSRID: executeStep worked!"<<std::endl;
    std::cout<<"Shape::getSRID: srid-string=|"<<srid<<"|"<<std::endl;
    //std::cout<<"Shape::getSRID: result-string0=|"<<(const char *)query.getColumn(0)<<"|"<<std::endl;
    //std::cout<<"Shape::getSRID: result-string1=|"<<(const char *)query.getColumn(1)<<"|"<<std::endl;
    //std::cout<<"Shape::getSRID: result-string2=|"<<(const char *)query.getColumn(2)<<"|"<<std::endl;
    //std::cout<<"Shape::getSRID: result-string3=|"<<(const char *)query.getColumn(3)<<"|"<<std::endl;
    //std::cout<<"Shape::getSRID: result-string4=|"<<(const char *)query.getColumn(4)<<"|"<<std::endl;
    //std::cout<<"Shape::getSRID: result-string5=|"<<(const char *)query.getColumn(5)<<"|"<<std::endl;
#endif


  }//endif

#ifdef DEBUG2
  std::cout<<"Shape::getSRID: srid-string=|"<<srid<<"|  srid-as-int=|"<<std::stoi(srid)<<"|"<<std::endl;
#endif
  return std::stoi(srid);
  
}//end-function: getSRID


//----------------------------------------------------------------------
//----------------------------------------------------------------------
int Shape::getDims()
{
  // the function context scopes this "Statement"
  SQLite::Statement   query(*db, "SELECT coord_dimension from geometry_columns WHERE f_table_name = 'shape'");
  
  // just once:
  std::string dims("2"); // 2D is default
  if(query.executeStep()){
    dims = (const char *)query.getColumn(0);

#ifdef DEBUG2
    std::cout<<"Shape::getDims: executeStep worked!"<<std::endl;
    std::cout<<"Shape::getDims: dims-string=|"<<dims<<"|"<<std::endl;
    //std::cout<<"Shape::getSRID: result-string0=|"<<(const char *)query.getColumn(0)<<"|"<<std::endl;
    //std::cout<<"Shape::getSRID: result-string1=|"<<(const char *)query.getColumn(1)<<"|"<<std::endl;
    //std::cout<<"Shape::getSRID: result-string2=|"<<(const char *)query.getColumn(2)<<"|"<<std::endl;
    //std::cout<<"Shape::getSRID: result-string3=|"<<(const char *)query.getColumn(3)<<"|"<<std::endl;
    //std::cout<<"Shape::getSRID: result-string4=|"<<(const char *)query.getColumn(4)<<"|"<<std::endl;
    //std::cout<<"Shape::getSRID: result-string5=|"<<(const char *)query.getColumn(5)<<"|"<<std::endl;
#endif


  }//endif

  if( (dims=="2") || (dims=="XY") || (dims=="xy" ) ) {
    return 2;
  } else if ((dims=="3") || (dims=="XYZ") || (dims=="xyz" )) {
    return 3;
  }//endif

  throw std::runtime_error("Shape::getDims: unknown dims string:"+dims);
  
}//end-function: getDims






  
} //end: geostar