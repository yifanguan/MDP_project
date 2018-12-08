/* M_PI is defined in math.h in the case of Microsoft Visual C++, Solaris,
 * et. al.
 */
#if defined(_MSC_VER)
#define _USE_MATH_DEFINES
#endif 

#include <string>
#include <iostream>
//#include <cairommconfig.h>
#include <cairomm/cairomm.h>
#include <cairomm/context.h>
#include <cairomm/surface.h>

#include <cstdint>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include "geostar.hpp"
#include "boost/filesystem.hpp"

#include "IFile.hpp"
int main()
{
    std::ifstream myfile;
    myfile.open("output");
    std::string info;
    std::getline(myfile, info);
    std::istringstream is(info);
    std::getline(is, info, '(');
    std::cout << info << std::endl;
    std::getline(is, info, ')');
    std::cout << info << std::endl;
    // assume LINESTRING
    info += ",";
    std::vector<std::pair<double, double>> points;
    int i = 0;
    int j = i + 1;
    int len = static_cast<int>(info.size());
    double x = 0;
    double y = 0;
    while (j < len) {
        if (info[j] == ' ') {
            x = stod(info.substr(i, j-i));
            i = j + 1;
            j = i + 1;
        }
        else if (info[j] == ',') {
            y = stod(info.substr(i, j-i));
            i = j + 2;
            j = i + 1;
            points.push_back(std::make_pair(x, y));
        }
        else {
            ++j;
        }
    }
    // for (int i  = 0; i < static_cast<int>(points.size()); ++i) {
    //     std::cout << points[i].first << " " << points[i].second << std::endl;
    // }
    int height = 400;
    int width = 600;
    // int stride = cairo_format_stride_for_width (Cairo::Cairo_FORMAT_ARGB32, width);
    // unsigned char *buffer = malloc(stride * height);
    auto surface =
        Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, width, height);
    unsigned char* buffer = surface->get_data();

    auto cr = Cairo::Context::create(surface);

    // cr->save(); // save the state of the context
    // cr->set_source_rgb(0.86, 0.85, 0.47);
    // cr->paint();    // fill image with the color
    // cr->restore();  // color is back to black now

    cr->save();
    // draw a border around the image
    // cr->set_line_width(20.0);    // make the line wider
    // cr->rectangle(0.0, 0.0, surface->get_width(), surface->get_height());
    // cr->stroke();

    // cr->set_source_rgba(0.0, 0.0, 0.0, 0.7);
    // // draw a circle in the center of the image
    // cr->arc(surface->get_width() / 2.0, surface->get_height() / 2.0, 
    //         surface->get_height() / 4.0, 0.0, 2.0 * M_PI);
    // cr->stroke();

    // draw a diagonal line
    for (int i = 0; i < static_cast<int>(points.size()-1); ++i) {
        cr->move_to(points[i].first, points[i].second);
        cr->line_to(points[i+1].first, points[i+1].second);
    }
    cr->line_to(points.back().first, points.back().second);
    // cr->move_to(surface->get_width() / 4.0, surface->get_height() / 4.0);
    // cr->line_to(surface->get_width() * 3.0 / 4.0, surface->get_height() * 3.0 / 4.0);
    cr->stroke();
    // cr->restore();

    std::string filename = "image1.png";
    surface->write_to_png(filename);

    std::cout << "Wrote png file \"" << filename << "\"" << std::endl;

    GeoStar::File *f = new GeoStar::File("a1.h5", "existing");

    // GeoStar::Image *img = f->create_image("image1.png");
    // const int nx = 512;
    // const int ny = 512;
    // GeoStar::Raster *ras = img->create_raster("test", GeoStar::INT8U, nx , ny);

    // delete f;

    GeoStar::Image *img;
    img = f->open_image("cairo_new");

    GeoStar::Raster *ras;
   
    const GeoStar::Slice outSlice(0,0,100,50);
    std::vector<char> buffer2(buffer, buffer + surface->get_height() * surface->get_stride());
    ras->write(outSlice, buffer2);

    delete ras;
    delete img;
}