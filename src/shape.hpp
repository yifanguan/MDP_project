// Shape.hpp
//
// by Haoran Xiao, Mar 26, 2018
// based on Raster.hpp
//
// modified by Leland Pierce, June&July, 2018
//----------------------------------------
#ifndef SHAPE_HPP_
#define SHAPE_HPP_

#include <string>
#include <vector>
#include <memory>


#include "H5Cpp.h"
#include <sqlite3.h>

#include "SQLiteCpp.hpp"


#include "Exceptions.hpp"
#include "attributes.hpp"

extern "C" {
sqlite3_vfs *sqlite3_HDFCPPvfs(void);
#include "spatialite.h"
#include "spatialite_private.h"
}

#include "gdal_priv.h"
#include "SimpleFeatures.hpp"


#include <iostream>
//#define DEBUG2

namespace GeoStar {
  class Vector;

  class Shape {

  private:
	std::string shapename;
	std::string shapetype;
    //sqlite3 * db; // handle opened on construction, closed on destruction
      SQLite::Database  *db;

    void create(Vector *vec, const std::string &name, 
                const std::string &descriptor, const int srid, const int dims);
    void open(Vector *vec, const std::string &name);
    void addSpatialReferenceSystems();
    void createShapeTable(const int srid, const int dims);
    
    void *cache;
    int dims;
    int srid;
    
  public:
    //SQLite::Database  *db;
      //std::shared_ptr<SQLite::Database> dbPtr;
      
    H5::DataSet *shapeobj;
    
    /** \brief Shape constructor allows one to create a new GeoStar shape or open an existing one.

        Shapes hold collections of points, lines and polygons.
        The Shape constructor is used to make a new shape, or open an existing shape in the HDF5 GeoStar group format.
        The new shape is created empty, except for the needed metadata to define it as a GeoStar shape.
        In both cases, the shape is also opened, and a valid Shape object is returned.
        
        \see open, close
        
        \param[in] vec
        This is a GeoStar::Vector pointer, it is used to reference the Vector to create the dataset
        
        \param[in] name
        This is a string, set by the user, that holds the desired name of the new shape.
        
        \returns
        A valid Shape object on success.
        
        \par Exceptions
        Exceptions that may be raised by this method:
        ShapeOpenErrorException
        
        \par Example
        Let's say a user wants to open a shape named "shape_1":
        \code
        #include "geostar.hpp"
        #include <string>
        using namespace std;
        int main() 
        {
          GeoStar::File *file;
          try {
              file = new GeoStar::File("sirc_raco","new");
          }//end-try 
          catch (...) {
              cerr << "GeoStar::File creation failure"<<endl;
          }//end-catch
          
          GeoStar::Vector *vec;
          try {
              vec = file->create_vector("vec_group_1", "group 1 description");
          }//end-try 
          catch (...) {
              cerr << "GeoStar::Vector creation failure"<<endl;
          }//end-catch
          GeoStar::Shape *shp;
          try {
              shp = vec->open_shape("shape_1"); // this calls the Shape constructor
          }//end-try 
          catch (...) {
              cerr << "GeoStar::Shape open failure"<<endl;
          }//end-catch          
          
        }//end-main
        \endcode
        
    \par Details
       The Shape constructor should only be called by the GeoStar::Vector function open_shape().
       The HDF5 shape attribute named "object_type" is created for a new shape and filled 
       with value "geostar::shape".
       For existing vectors, this attribute must exist and have this value, or it is not 
       a GeoStar shape, and an exception is thrown.
  
  */
  Shape(Vector *vec, const std::string &name);

  /** \brief Shape constructor allows one to create a new GeoStar shape.

   The Shape constructor is used to make a new shape.
   The new shape is created empty, except for the needed metadata to define it as a GeoStar shape.

   \see open, close

  \param[in] vec
   This is a GeoStar::Vector pointer, it is used to reference the Vector to create the dataset

   \param[in] name
       This is a string, set by the user, that holds the desired name of the new shape.
    
   \param[in] descriptor
      This is a string, set by the user, that holds the description of the new shape.

   \returns
       A valid Shape object on success.

   \par Exceptions
       Exceptions that may be raised by this method:
       ShapeExistsException

   \par Example
       Let's say a user wants to create a shape named "shape_1":
       \code
       #include "geostar.hpp"
       #include <string>
       using namespace std;
       int main() 
       {
           GeoStar::File *file;
     try {
               file = new GeoStar::File("sirc_raco","new");
     }//end-try 
     catch (...) {
         cerr << "GeoStar::File creation failure"<<endl;
     }//end-catch

           GeoStar::Vector *vec;
     try {
               vec = file->create_vector("vec_group_1", "group 1 description");
     }//end-try 
     catch (...) {
         cerr << "GeoStar::Vector creation failure"<<endl;
     }//end-catch
          GeoStar::Shape *shp;
     try {
               shp = vec->create_shape("shape_1", "shape 1 description"); // this calls the Shape constructor
     }//end-try 
     catch (...) {
         cerr << "GeoStar::Shape create failure"<<endl;
     }//end-catch          

       }//end-main
       \endcode

    \par Details
       The Shape constructor should only be called by the GeoStar::Vector function create_shape().
       The HDF5 shape attribute named "object_type" is created for a new shape and filled 
       with value "geostar::shape".
       For existing vectors, this attribute must exist and have this value, or it is not 
       a GeoStar shape, and an exception is thrown.
  */
  Shape(Vector *vec, const std::string &name, const std::string &descriptor);

  Shape(Vector *vec, const std::string &name, 
        const std::string &descriptor, const int srid, const int dims);


    //SQLite::Database  *getHandle() { return db;}

    // cleans up the H5::DataSet object 
    inline ~Shape() {
        std::cerr << "here A\n";
      if(db){
          std::cerr << "here b\n";
      delete db;
          //dbPtr = NULL;
          std::cerr << "here c\n";
      spatialite_cleanup_ex (cache);
          std::cerr << "here d\n";
      spatialite_shutdown();
          std::cerr << "here e\n";
      delete shapeobj;
          std::cerr << "here f\n";
      }
        std::cerr << "here g\n";

    }//end-func: ~Shape

  // returns name of shape as string
    std::string get_name() {return shapename;}



    // write the "object_type" attribute
    inline void write_object_type(const std::string &value) {
      GeoStar::write_object_type((H5::H5Object *)shapeobj,value);
    }
    inline void append_object_type(const std::string &value) {
      GeoStar::append_object_type((H5::H5Object *)shapeobj,value);
    }
    
    // read the "object_type" attribute
    std::string read_object_type() const {
      return GeoStar::read_object_type((H5::H5Object *)shapeobj);
    }



    void addPoint(const double x, const double y);
    void addPoint(const double x, const double y, const double z);
    
    template <typename T>
    void addPoint(const SimpleFeatures::Point<T> pt);
     
    template <typename T>
    void addPoint(const SimpleFeatures::Point3D<T> pt);
    

    template <typename T>
    void addLine(const SimpleFeatures::Line<T> line);
    
    template <typename T>
    void addLine(const SimpleFeatures::Line3D<T> line);
    
    template <typename T>
    void addPolygon(const SimpleFeatures::Polygon<T> poly);
    
    template <typename T>
    void addPolygon(const SimpleFeatures::Polygon3D<T> poly);

    void getGeometries();
    
    /** \brief Shape::getWithin retrieves a vector of (pk, WKT string) pairs within the geometry specified by user.

   \param[in] text
       This is a string, set by the user, that holds the WKT string of a geometry.
    
   \returns
       A vector of pairs where the first is primary key, and the second is the corresponding geometry represented by WKT string.
  */
    std::vector<std::pair<int, std::string>> getWithin(std::string text);

    /** \brief Shape::getLength retrieves a vector of (pk, length) pairs with type LINESTRING

   \returns
       A vector of pairs where the first element is primary key, and the second is the corresponding length.
  */
    std::vector<std::pair<int, double>> getLength();

    /** \brief Shape::getPerimeter retrieves a vector of (pk, perimeter) pairs with type POLYGON

   \returns
       A vector of pairs where the first element is primary key, and the second is the corresponding perimeter.
  */
    std::vector<std::pair<int, double>> getPerimeter();

  /** \brief Shape::getPerimeter retrieves a vector of (pk, area) pairs with type POLYGON

   \returns
       A vector of pairs where the first element is primary key, and the second is the corresponding area.
  */
    std::vector<std::pair<int, double>> getArea();

  /** \brief Shape::getBoundary retrieves a vector of (pk, boundary) pairs of each geometry in shape

   \returns
       A vector of pairs where the first element is primary key, and the second is the corresponding boundary represented as WKT string.
  */
    std::vector<std::pair<int, std::string>> getBoundary();

  /** \brief Shape::getEnvelope retrieves a vector of (pk, envelope) pairs of each geometry in shape.
      \brief Envelope is the rectangle bounding as a polygon.

   \returns
       A vector of pairs where the first element is primary key, and the second is the corresponding envelope represented as WKT string.
  */
    std::vector<std::pair<int, std::string>> getEnvelope();

  /** \brief Shape::getExpand retrieves a vector of (pk, boundary) pairs of each geometry in shape 
      \brief after expanding by amount parameter in all directions

    \param[in] amount
       This is a int, set by the user

   \returns
       A vector of pairs where the first element is primary key, and the second is the corresponding boundary represented as WKT string.
  */
    std::vector<std::pair<int, std::string>> getExpand(int amount);

  /** \brief Shape::getCentroid retrieves a vector of (pk, centroid) pairs of each geometry in shape 

   \returns
       A vector of pairs where the first element is primary key, and the second is the corresponding centroid represented as WKT string.
  */
    std::vector<std::pair<int, std::string>> getCentroid();

  /** \brief Shape::getSimplify retrieves a vector of (pk, simplified geometry) pairs of each geometry in shape 
      \brief by applying Douglas-Peuker algorithm for each geometry in shape
   \returns
       A vector of pairs where the first element is primary key, and the second is the corresponding simplified geometry represented as WKT string.
  */
    std::vector<std::pair<int, std::string>> getSimplify();


    // another series of function by passing PK as parameter
    // and do query on the specific geometry in shape
    // return the result of the query

    /** \brief Shape::getSimplify retrieves a simplified geometry by applying Douglas-Peuker algorithm for each geometry in shape

      \param[in] PK_id
       This is a int, set by the user, representing the primary key
      \returns
         The corresponding simplified geometry with the primary key represented as WKT string.
    */
    std::string getSimplify(int PK_id);

    /** \brief Shape::getCentroid retrieves the centroid of the geometry with the primary key

      \param[in] PK_id
       This is a int, set by the user, representing the primary key
      \returns
         The corresponding centroid of the geometry with the primary key represented as WKT string.
    */
    std::string getCentroid(int PK_id);

    /** \brief Shape::getExpand retrieves the rectangle boundary of the geometry with the primary key after expanding by amount parameter in all directions

      \param[in] PK_id
       This is a int, set by the user, representing the primary key
     \param[in] amount
       This is a int, set by the user
      \returns
         The corresponding rectangle boundary of the geometry with the primary key represented as WKT string.
    */
    std::string getExpand(int PK_id, int amount);

    /** \brief Shape::getExpand retrieves the envelope of the geometry with the primary key
        \brief Envelope is the rectangle bounding as a polygon.

      \param[in] PK_id
       This is a int, set by the user, representing the primary key
      \returns
         The corresponding envelop of the geometry with the primary key represented as WKT string.
    */
    std::string getEnvelope(int PK_id);

    /** \brief Shape::getExpand retrieves the boundary of the geometry with the primary key.
      
      \param[in] PK_id
       This is a int, set by the user, representing the primary key.
      \returns
         The corresponding boundary of the geometry with the primary key represented as WKT string.
    */
    std::string getBoundary(int PK_id);
    
    /** \brief Shape::getArea retrieves the area of the geometry with the primary key.
      
      \param[in] PK_id
       This is a int, set by the user, representing the primary key.
      \returns
         The corresponding area of the geometry with the primary key.
    */
    double getArea(int PK_id);

    /** \brief Shape::getPerimeter retrieves the perimeter of the geometry with the primary key.
      
      \param[in] PK_id
       This is a int, set by the user, representing the primary key.
      \returns
         The corresponding Perimeter of the geometry with the primary key.
    */
    double getPerimeter(int PK_id);

    /** \brief Shape::getLength retrieves the length of the geometry with the primary key
      
      \param[in] PK_id
       This is a int, set by the user, representing the primary key.
      \returns
         The corresponding length of the geometry with the primary key.
    */
    double getLength(int PK_id);

    /** \brief Shape::getGeometry retrieves a geometry with PK = PK_id from shape.
      \param[in] PK_id
       This is a int, set by the user, representing the primary key.
      \returns
         The corresponding geometry with the primary key as WKT string.
    */
    std::string getGeometry(int PK_id);


    /** \brief Shape::writeGeometryToFile writes a geometry specified by parameter geometry to a file in the format of WKT string.
      \param[in] geometry
       This is a string, set by the user, representing a geometry.
      \param[in] filename
       This is a string, set by the user, representing the name the file to write.
    */
    void writeGeometryToFile(std::string geometry, std::string filename);
    

  /** \brief Shape::get_column_as_string retrieves the data in a given column as a vector of strings

  get_column_as_string() is given a valid column name in the table, which then adds all the row's data
  into a vector of strings

   \see get_geoms_as_WKT

   \param[in] colname
       This is a string, set by the user, that holds the desired name of the column to be queried.
    
   \returns
       A vector of strings.

   \par Exceptions
       ShapeReadErrorException

   \par Example
       Let's say a user wants to get the area of each geometry in the "reg2001" shapefile:
       \code
       #include "geostar.hpp"
       #include <string>
       #include <vector>
       using namespace std;
       int main() 
       {
           GeoStar::File *file;
     try {
               file = new GeoStar::File("test_h5","new");
     }//end-try 
     catch (...) {
         cerr << "GeoStar::File creation failure"<<endl;
     }//end-catch

           GeoStar::Vector *vec;
     try {
               vec = file->create_vector("vector1", "vector 1 description");
     }//end-try 
     catch (...) {
         cerr << "GeoStar::Vector creation failure"<<endl;
     }//end-catch

          GeoStar::Shape *shp;
      try {
        shape = vec->read_shape("reg2001_s.shp", "reg2001");
      }
      catch(...) {
        cout << "Failed vector import" << endl;
      }
      try {
        vector<string> tmp = shape->get_column_as_string("ST_AREA(geom)");
        for (auto & i : tmp)
          cout << i << endl;
      }
      catch(...) {
        cout << "Failed get_column_as_string" << endl;
      }
       }//end-main
       \endcode

    \par Details
       The function throws an exception when the underlying SQLite fails, that can be caused by
       bad table or bad column name.
  */
    std::vector<std::string> get_column_as_string(const std::string & colname);


  /** \brief Shape::get_geoms_as_WKT retrieves the geometry column as a vector of strings

  get_geoms_as_WKT() converts the geometry column into WKT and returns them in a vector as strings

   \see get_column_as_string

   \returns
       A vector of strings.

   \par Exceptions
       ShapeReadErrorException

   \par Example
       Let's say a user wants to get the WKT in the "reg2001" shapefile:
       \code
       #include "geostar.hpp"
       #include <string>
       #include <vector>
       using namespace std;
       int main() 
       {
           GeoStar::File *file;
     try {
               file = new GeoStar::File("test_h5","new");
     }//end-try 
     catch (...) {
         cerr << "GeoStar::File creation failure"<<endl;
     }//end-catch

           GeoStar::Vector *vec;
     try {
               vec = file->create_vector("vector1", "vector 1 description");
     }//end-try 
     catch (...) {
         cerr << "GeoStar::Vector creation failure"<<endl;
     }//end-catch

          GeoStar::Shape *shp;
      try {
        shape = vec->read_shape("reg2001_s.shp", "reg2001");
      }
      catch(...) {
        cout << "Failed vector import" << endl;
      }
      try {
        vector<string> tmp = shape->get_geoms_as_WKT();
        for (auto & i : tmp)
          cout << i << endl;
      }
      catch(...) {
        cout << "Failed get_column_as_string" << endl;
      }
       }//end-main
       \endcode

    \par Details
       The function throws an exception when the underlying SQLite fails, that can be caused by a
       bad table.
  */  
    std::vector<std::string> get_geoms_as_WKT();



  int exportIAUToProj4( const OGRSpatialReference &asr, char ** ppszProj4 ) const;

    void updateSRID(const int srid);
    int getSRID();
    int getDims();

      inline SQLite::Database* getDbHandle() {
      //inline std::shared_ptr<SQLite::Database> getDbHandle() {
          /*
          int* p = new int (10);
          std::shared_ptr<int> a (p);
          
          if (a.get()==p)
              std::cout << "a and p point to the same location\n";
           */
          
          //std::shared_ptr<SQLite::Database> dbPtr(db);
          
          //return dbPtr;
          return db;
      }

      
      
  }; // end class: Shape
  
}// end namespace GeoStar


#endif //SHAPE_HPP_