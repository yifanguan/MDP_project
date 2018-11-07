// testcppvector.cpp
//
// by Haoran Xiao, Mar 26, 2018
//
//---------------------------------------------------------
#include <string>
#include <iostream>
#include <cstdint>
#include <vector>

#include "geostar.hpp"

#include "boost/filesystem.hpp"

#include "IFile.hpp"

int main() {

    // delete output file if already exists
    boost::filesystem::path p("testcpp.h5");
    boost::filesystem::remove(p);
    


	// test vector creation, opening, closing
	GeoStar::File * file = new GeoStar::File("testcpp.h5","new");
	GeoStar::Vector * vec = file->create_vector("vector1", "vector1 description");
	delete vec;
	try {
		vec = file->create_vector("vector1", "vector1 duplicate description");
	}
	catch(...) {
		std::cout << "Caught exception as expected: 1" << std::endl;
	}
	vec = file->open_vector("vector1");
	delete vec;

        std::cout<<"main: 1"<<std::endl;

	// test shape creation, opening, closing
	vec = file->open_vector("vector2");

        std::cout<<"main: 2====================================="<<std::endl;

	GeoStar::Shape * shape = vec->create_shape("shape1", "shape 1 description");



        std::cout<<"main: 3==================================="<<std::endl;



	delete shape;

        std::cout<<"main: 3.1====================================="<<std::endl;

	try {
		shape = vec->create_shape("shape1", "shape 1 description");
	}
	catch(...) {
		std::cout << "Caught exception as expected: 2" << std::endl;
	}
        std::cout<<"main: 3.2====================================="<<std::endl;

	shape = vec->open_shape("shape1");
        std::cout<<"main: 3.2.0.2====================================="<<std::endl;
        shape->addPoint(13.5, 26.9);
        shape->addPoint(413.5, 826.9);
        SimpleFeatures::Point<double> aa(45.8, 129.45);
        shape->addPoint(aa);

        //SimpleFeatures::Point3D<double> aa3d(4566.8, 129.45, 94.12);
        //shape->addPoint(aa3d);


        SimpleFeatures::Line<double> ab;
        ab.resize(3);
        ab[0]=aa;
        aa.x=37.8; aa.y=12.54;
        ab[1]=aa;
        aa.x=307.8; aa.y=112.54;
        ab[2]=aa;
        shape->addLine(ab);
        std::cout<<"main: 3.2.1====================================="<<std::endl;

        SimpleFeatures::Ring<double> ac(ab);   // ring guarantees the "line" is closed (1st pt = last pt)
        std::cout<<"main: 3.2.1.1====================================="<<std::endl;
        SimpleFeatures::Polygon<double> adf;
        adf.push_back(ac);  // FIRST IS ALWAYS THE OUTER RING!
        adf.push_back(ac);  //  an inner ring
        std::cout<<"main: 3.2.2====================================="<<std::endl;
        shape->addPolygon(adf);
        std::cout<<"main: 3.2.3====================================="<<std::endl;
        

        shape->getGeometries();
	delete shape;
        std::cout<<"main: 3.2.5====================================="<<std::endl;

	shape = vec->open_shape("shape1");
        std::cout<<"main: 3.2.6====================================="<<std::endl;
	delete shape;
        std::cout<<"main: 3.2.7====================================="<<std::endl;

        std::cout<<"===========testing within======================="<<std::endl;
        shape = vec->create_shape("shape10", "shape for testing within");
	shape->addPoint(0.5, 0.5);
	shape->addPoint(4, 4);
	shape->addPoint(100, 375);

        SimpleFeatures::Line<double> aab;
        aab.resize(5);
	SimpleFeatures::Point<double> aaa(0, 0);
        aab[0]=aaa;
        aaa.x=1; aaa.y=0;
        aab[1]=aaa;
	aaa.x=1; aaa.y=1;
        aab[2]=aaa;
	aaa.x=0; aaa.y=1;
        aab[3]=aaa;
	aaa.x=0; aaa.y=0;
        aab[4]=aaa;
        shape->addLine(aab);
	SimpleFeatures::Ring<double> aac(aab);
	SimpleFeatures::Polygon<double> aaf;
	aaf.push_back(aac);
	shape->addPolygon(aaf);
	
	std::string polygonText = "POLYGON((0 0, 0 1, 1 1, 1 0, 0 0))";
	std::vector<std::pair<int, std::string>> res = shape->getWithin(polygonText);
	std::cout<<"======begin outputting result from getWithin===="<<std::endl;
	for (int i = 0; i < res.size(); ++i) {
	    std::cout << "getWithin::PK: " << res[i].first << ", geoText: " << res[i].second << std::endl;
	}
	
	std::cout<<"===========testing length======================="<<std::endl;
        shape = vec->create_shape("shape11", "shape for testing length");
	shape->addPoint(1, 1);
	SimpleFeatures::Line<double> line;
        line.resize(3);
	SimpleFeatures::Point<double> pt(0, 0);
        line[0]=pt;
        pt.x=1; pt.y=0;
        line[1]=pt;
	pt.x=1; pt.y=1;
        line[2]=pt;
	shape->addLine(line);
	std::cout<<"======begin outputting result from getLength===="<<std::endl;
	std::vector<std::pair<int, double>> res2 = shape->getLength();
	for (size_t i = 0; i < res2.size(); ++i) {
	    std::cout << "getLength::PK: " << res2[i].first << ", geoLength: " << res2[i].second << std::endl;
	}

	std::cout<<"===========testing perimeter======================="<<std::endl;
        shape = vec->create_shape("shape12", "shape for testing perimeter");
	//shape->addPoint(10, 5);
	SimpleFeatures::Line<double> line2;
        line2.resize(3);
	SimpleFeatures::Point<double> pt2(0, 0);
        line2[0]=pt2;
        pt2.x=2; pt2.y=0;
        line2[1]=pt2;
	pt2.x=1; pt2.y=1;
        line2[2]=pt2;
	
	shape->addLine(line2);
        SimpleFeatures::Ring<double> ring(line2);
	SimpleFeatures::Polygon<double> polygon;
	polygon.push_back(ring);
	shape->addPolygon(polygon);

	std::cout<<"======begin outputting result from getPeremeter===="<<std::endl;
	std::vector<std::pair<int, double>> res3 = shape->getPerimeter();
	for (size_t i = 0; i < res3.size(); ++i) {
	    std::cout << "getPeremeter::PK: " << res3[i].first << ", geoPeremeter: " << res3[i].second << std::endl;
	}

	std::cout<<"======begin outputting result from getArea======"<<std::endl;
	std::vector<std::pair<int, double>> res4 = shape->getArea();
	for (size_t i = 0; i < res4.size(); ++i) {
	    std::cout << "getArea::PK: " << res4[i].first << ", getArea: " << res4[i].second << std::endl;
	}

	std::cout<<"======begin outputting result from getBoundary======"<<std::endl;
	std::vector<std::pair<int, std::string>> res5 = shape->getBoundary();
	for (size_t i = 0; i < res5.size(); ++i) {
	    std::cout << "getBoundary::PK: " << res5[i].first << ", getBoundary: " << res5[i].second << std::endl;
	}

	std::cout<<"======testing getBoundary Return new shape===="<<std::endl;

	GeoStar::Shape* res_shape = vec->create_shape("result shape", "shape for storing results of boundaries");
	shape->getBoundaryReturnShape(res_shape);
	res_shape->getGeometries();

	std::cout<<"======begin outputting result from getEnvelope======"<<std::endl;
	std::vector<std::pair<int, std::string>> res6 = shape->getEnvelope();
	for (size_t i = 0; i < res6.size(); ++i) {
	    std::cout << "getEnvelope::PK: " << res6[i].first << ", getEnvelope: " << res6[i].second << std::endl;
	}

	std::cout<<"======testing getEnvelope Return new shape===="<<std::endl;

	GeoStar::Shape* res_shape2 = vec->create_shape("result shape2", "shape for storing results of envelope");
	shape->getEnvelopeReturnShape(res_shape2);
	res_shape->getGeometries();

	std::cout<<"======begin outputting result from getExpand======"<<std::endl;
	std::vector<std::pair<int, std::string>> res7 = shape->getExpand(2);
	for (size_t i = 0; i < res7.size(); ++i) {
	    std::cout << "getExpand::PK: " << res7[i].first << ", getExpand: " << res7[i].second << std::endl;
	}

	std::cout<<"======testing getExpand Return new shape===="<<std::endl;

	GeoStar::Shape* res_shape3 = vec->create_shape("result shape3", "shape for storing results of expand");
	shape->getExpandReturnShape(res_shape3, 2);
	res_shape->getGeometries();

	std::cout<<"======begin outputting result from getCentroid======"<<std::endl;
	std::vector<std::pair<int, std::string>> res8 = shape->getCentroid();
	for (size_t i = 0; i < res8.size(); ++i) {
	    std::cout << "getCentroid::PK: " << res8[i].first << ", getCentroid: " << res8[i].second << std::endl;
	}

	std::cout<<"======testing getCentroid Return new shape===="<<std::endl;

	GeoStar::Shape* res_shape4 = vec->create_shape("result shape4", "shape for storing results of centroid");
	shape->getCentroidReturnShape(res_shape4);
	res_shape->getGeometries();
        


	std::cout<<"===========testing simplify======================="<<std::endl;
	shape = vec->create_shape("shape13", "shape for testing simplify");
	shape->addPoint(3.3, 5);
	SimpleFeatures::Line<double> line3;
	line3.resize(4);
	SimpleFeatures::Point<double> pt3(0, 0);
	line3[0]=pt3;
        pt3.x=1; pt3.y=0.01;
	line3[1]=pt3;
        pt3.x=2; pt3.y=0;
        line3[2]=pt3;
	pt3.x=1; pt3.y=1;
        line3[3]=pt3;
	shape->addLine(line3);
	
	std::cout<<"======begin outputting result from getSimplify======"<<std::endl;
	std::vector<std::pair<int, std::string>> res9 = shape->getSimplify();
	for (size_t i = 0; i < res9.size(); ++i) {
	    std::cout << "getSimplify::PK: " << res9[i].first << ", getSimplify: " << res9[i].second << std::endl;
	}

	std::cout<<"======testing getSimplify Return new shape===="<<std::endl;

	GeoStar::Shape* res_shape5 = vec->create_shape("result shape5", "shape for storing results of Simplify");
	shape->getSimplifyReturnShape(res_shape5);
	res_shape->getGeometries();

	std::cout<<"======testing getGeometry====================="<<std::endl;
	std::string geo = shape->getGeometry(2);
	std::cout<<geo<<std::endl;
	shape->writeGeometryToFile(geo, "lineString");

	std::cout<<"======testing deleteRow====================="<<std::endl;
	std::cout<<"Before deleteRow()"<<std::endl;
	shape->getGeometries();
	shape->deleteRow(1);
	std::cout<<"After deleteRow()"<<std::endl;
	shape->getGeometries();
	std::cout<<"======testing movePoint====================="<<std::endl;
	shape = vec->create_shape("shape15", "shape for testing movePoint");
	shape->addPoint(1.0, 2.0);
	shape->movePoint(1, 2.0, 3.0);
	shape->getGeometries();
	std::cout<<"======testing deletePointInLinestring=========="<<std::endl;
	SimpleFeatures::Line<double> line4;
	SimpleFeatures::Point<double> pt4(0, 0);
	line4.resize(3);
	line4[0] = pt4;
	pt4.x = 1;
	pt4.y = 2;
	line4[1] = pt4;
	pt4.x = 3;
	pt4.y = 4;
	line4[2] = pt4;
	shape->addLine(line4);
	std::cout<<"Before deleting a point in linestring"<<std::endl;
	shape->getGeometries();
	std::cout<<"After deleting a point in linestring"<<std::endl;
	std::string point_to_delete = " 1 2,";
	shape->deletePointInLinestring(2, point_to_delete);
	shape->getGeometries();

	std::cout<<"======testing deletePointInPOLYGON=========="<<std::endl;
	shape = vec->create_shape("shape16", "shape for testing deletePointInPOLYGON");
	shape->addPoint(0.5, 0.5);
	shape->addPoint(4, 4);
	shape->addPoint(100, 375);

        SimpleFeatures::Line<double> line5;
        line5.resize(5);
	SimpleFeatures::Point<double> point2(0, 0);
        line5[0]=point2;
        point2.x=1; point2.y=0;
        line5[1]=point2;
	point2.x=1; point2.y=1;
        line5[2]=point2;
	point2.x=0; point2.y=1;
        line5[3]=point2;
	point2.x=0; point2.y=0;
        line5[4]=point2;
        shape->addLine(line5);
	SimpleFeatures::Ring<double> ring3(line5);
	SimpleFeatures::Polygon<double> polygon2;
	polygon2.push_back(ring3);
	shape->addPolygon(polygon2);
	std::cout<<"Before deleting a point in POLYGON"<<std::endl;
	shape->getGeometries();
	std::cout<<"After deleting a point in POLYGON"<<std::endl;
	std::string point_to_delete2 = " 0 0,";
	shape->deletePointInPolygon(5, point_to_delete2);
	shape->getGeometries();

	std::cout<<"======testing deleteRingInPOLYGON=========="<<std::endl;
	shape = vec->create_shape("shape17", "shape for testing deleteRingInPOLYGON");
	shape->addPoint(0.5, 0.5);
	shape->addPoint(4, 4);
	shape->addPoint(100, 375);

        SimpleFeatures::Line<double> line6;
        line6.resize(5);
	SimpleFeatures::Point<double> point3(0, 0);
        line6[0]=point3;
        point3.x=1; point3.y=0;
        line6[1]=point3;
	point3.x=1; point3.y=1;
        line6[2]=point3;
	point3.x=0; point3.y=1;
        line6[3]=point3;
	point3.x=0; point3.y=0;
        line6[4]=point3;
        shape->addLine(line6);
	SimpleFeatures::Ring<double> ring4(line6);
	SimpleFeatures::Polygon<double> polygon3;
	polygon3.push_back(ring4);
	shape->addPolygon(polygon3);
	std::cout<<"Before deleting a ring in POLYGON"<<std::endl;
	shape->getGeometries();
	std::cout<<"After deleting a ring in POLYGON"<<std::endl;
	std::string ring_to_delete = " 1 0, 1 1, 0 1";
	shape->deletePointInPolygon(5, ring_to_delete);
	shape->getGeometries();

        std::cout<<"======testing makecopy=========="<<std::endl;
	shape = vec->create_shape("shape18", "shape for testing deleteRingInPOLYGON");
	shape->addPoint(0.5, 0.5);
	shape->addPoint(4, 4);
	shape->addPoint(100, 375);
	std::cout<<"Before making a copy"<<std::endl;
	shape->getGeometries();
	std::cout<<"After making a copy"<<std::endl;
	shape->makecopy(3); //copy a point
	shape->getGeometries();

	exit(-1);//debug

        std::cout<<"main: 3.3====================================="<<std::endl;
	shape = vec->open_shape("shape2");
	delete shape;
        std::cout<<"main: 3.4====================================="<<std::endl;
	vec->delete_shape("shape1");
	vec->delete_shape("shape2");
        std::cout<<"main: 3.5====================================="<<std::endl;


#ifdef NOTNOW
        // print all the srid's
	shape = vec->open_shape("shape1");


    { // scope the query, so it is deleted at end of scope.
      SQLite::Statement   query(*(shape->db), "SELECT * FROM spatial_ref_sys;");

      while (query.executeStep())
        {
          const char *srid   = query.getColumn(0);
          const char *name   = query.getColumn(3);
          const char *text1   = query.getColumn(4);
          const char *text2   = query.getColumn(5);
          
          std::cout  << srid << ", " << name << ", |" << text1<<"|" << std::endl;
        }//end-while
    } //end of scope for query.

	vec->delete_shape("shape1");
#endif // NOTNOW




	delete vec;


        std::cout<<"main: DONE"<<std::endl;
        exit(-1);



















        std::cout<<"main: 4==========================="<<std::endl;

	// test shape import

	try{ 
          vec = file->open_vector("vector1");
        }
        catch(...) {
          std::cout << "Failed vector open" << std::endl;
          exit(-1);
	}
       
   
#ifdef CRAP
        std::cout<<"main: 4.5: about to read_shape==========================="<<std::endl;

	//try {
		shape = vec->read_shape("reg2001_s.shp", "reg2001");
                //}
                //	catch(...) {
                //		shape = nullptr;
                //		std::cout << "Failed vector import" << std::endl;
                //         exit(-1);
                //	}
	delete shape;

        std::cout<<"main: 5==========================="<<std::endl;
        exit(-1); // for now.

	// test get_column_as_string
	shape = vec->open_shape("reg2001");
        std::vector<std::string> tmp = shape->get_column_as_string("REGIONE");
	for (auto & i : tmp)
		std::cout << i << std::endl;
	tmp = shape->get_column_as_string("ST_AREA(geom)");
	for (auto & i : tmp)
		std::cout << i << std::endl;


        std::cout<<"main: 6============================"<<std::endl;

	// test get_geoms_as_WKT
        std::vector<std::string> tmp2 = shape->get_geoms_as_WKT();

	// I would suggest piping the output to a file if you want to print this out
	// it gets quite long for even just 20 shapes
	for (auto & i : tmp2)
		std::cout << i << std::endl;
	delete shape;
#endif

	delete vec;
	delete file;
}// end-main