#define DEBUG
/* -*-c++-*-
 *  ----------------------------------------------------------------------------
 *
 *       AMAPmod: Exploring and Modeling Plant Architecture
 *
 *       Copyright 1995-2000 UMR Cirad/Inra Modelisation des Plantes
 *
 *       File author(s): C. Nouguier & F. Boudon (frederic.boudon@cirad.fr) nouguier
 *
 *       $Source$
 *       $Id$
 *
 *       Forum for AMAPmod developers    : amldevlp@cirad.fr
 *
 *  ----------------------------------------------------------------------------
 *
 *                      GNU General Public Licence
 *
 *       This program is free software; you can redistribute it and/or
 *       modify it under the terms of the GNU General Public License as
 *       published by the Free Software Foundation; either version 2 of
 *       the License, or (at your option) any later version.
 *
 *       This program is distributed in the hope that it will be useful,
 *       but WITHOUT ANY WARRANTY; without even the implied warranty of
 *       MERCHANTABILITY or FITNESS For A PARTICULAR PURPOSE. See the
 *       GNU General Public License for more details.
 *
 *       You should have received a copy of the GNU General Public
 *       License along with this program; see the file COPYING. If not,
 *       write to the Free Software Foundation, Inc., 59
 *       Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  ----------------------------------------------------------------------------
 */




#include "discretizer.h"
#include "merge.h"

#include <plantgl/pgl_geometry.h>
#include <plantgl/pgl_transformation.h>
#include <plantgl/pgl_container.h>
#include <plantgl/scenegraph/scene/shape.h>

#include <plantgl/math/util_math.h>

#ifdef GEOM_DEBUG
#include <plantgl/tool/timer.h>
#endif

PGL_USING_NAMESPACE
TOOLS_USING_NAMESPACE

using namespace std;

/* ----------------------------------------------------------------------- */


#define GEOM_DISCRETIZER_CHECK_CACHE(geom) \
  if (geom->isNamed()) { \
    Cache<ExplicitModelPtr>::Iterator _it = __cache.find(geom->getId()); \
    if (! (_it == __cache.end())) { \
       __discretization = ExplicitModelPtr(_it->second); \
      if(__discretization)return true; \
      else cerr << "Cache of Discretizer Error !" << endl; \
    }; \
  } else __discretization = 0;

#define GEOM_DISCRETIZER_CHECK_CACHE_WITH_TEX(geom) \
  if (geom->isNamed()) { \
    Cache<ExplicitModelPtr>::Iterator _it = __cache.find(geom->getId()); \
	if (! (_it == __cache.end()) && MeshPtr::Cast(_it->second)->hasTexCoordList()) { \
       __discretization = ExplicitModelPtr(_it->second); \
      if(__discretization)return true; \
      else cerr << "Cache of Discretizer Error !" << endl; \
    }; \
  } else __discretization = 0;


#define GEOM_DISCRETIZER_UPDATE_CACHE(geom) \
  if (geom->isNamed()) { \
    if(__discretization)__discretization->setName(geom->getName()); \
    __cache.insert(geom->getId(),__discretization); \
  }


#define GEOM_DISCRETIZER_TRANSFORM(transformed) \
  GEOM_DISCRETIZER_CHECK_CACHE(transformed); \
  if(transformed->getGeometry().isValid() &&  \
	(transformed->getGeometry())->apply(*this) && \
    __discretization.isValid()){ \
    __discretization = __discretization->transform(transformed->getTransformation()); \
    GEOM_DISCRETIZER_UPDATE_CACHE(transformed); \
  } \
  else { \
    __discretization = ExplicitModelPtr(); \
    return false; \
  }

/* ----------------------------------------------------------------------- */


Discretizer::Discretizer( ) :
    Action(),
    __cache(),
    __discretization(),
	__computeTexCoord(false){
}

Discretizer::~Discretizer( ) {
}

void Discretizer::clear( ) {
  __discretization = ExplicitModelPtr();
  __cache.clear();
}

const ExplicitModelPtr& Discretizer::getDiscretization( ) const {
  return __discretization;
}

/* ----------------------------------------------------------------------- */

bool Discretizer::process(Shape * Shape){
    GEOM_ASSERT(Shape);
    return (Shape->geometry->apply(*this));
}


/* ----------------------------------------------------------------------- */
bool Discretizer::process( Material * material ) {
  GEOM_ASSERT(material);
  // nothing to do
  __discretization = ExplicitModelPtr();
  return false;
}

bool Discretizer::process( ImageTexture * texture ) {
  GEOM_ASSERT(texture);
  // nothing to do
  __discretization = ExplicitModelPtr();
  return false;
}

bool Discretizer::process( MonoSpectral * monoSpectral ) {
  GEOM_ASSERT(monoSpectral);
    // nothing to do
  __discretization = ExplicitModelPtr();
  return false;
}

bool Discretizer::process( MultiSpectral * multiSpectral ) {
  GEOM_ASSERT(multiSpectral);
  // nothing to do
  __discretization = ExplicitModelPtr();
  return false;
}

/* ----------------------------------------------------------------------- */

bool Discretizer::process( AmapSymbol * amapSymbol ) {
  GEOM_ASSERT(amapSymbol);
  // nothing to do as amapSymbol has a cached representation of type of Mesh.
  __discretization = ExplicitModelPtr(amapSymbol);
  return true;
}

bool Discretizer::process( FaceSet * faceSet ) {
  GEOM_ASSERT(faceSet);
  // nothing to do as faceSet is already an ExplicitModel
  __discretization = ExplicitModelPtr(faceSet);
  return true;
}

bool Discretizer::process( PointSet * pointSet ) {
  GEOM_ASSERT(pointSet);
  // nothing to do as quadSet is already an ExplicitModel
  __discretization = ExplicitModelPtr(pointSet);
  return true;
}

bool Discretizer::process( Polyline * polyline ) {
  GEOM_ASSERT(polyline);
  // nothing to do as quadSet is already an ExplicitModel
  __discretization = ExplicitModelPtr(polyline);
  return true;
}

bool Discretizer::process( QuadSet * quadSet ) {
  GEOM_ASSERT(quadSet);
  // nothing to do as quadSet is already an ExplicitModel
  __discretization = ExplicitModelPtr(quadSet);
  return true;
}

bool Discretizer::process( TriangleSet * triangleSet ) {
  GEOM_ASSERT(triangleSet);
  // nothing to do as triangleSet is already an ExplicitModel
  __discretization = ExplicitModelPtr(triangleSet);
  return true;
}


/* ----------------------------------------------------------------------- */

bool Discretizer::process( AsymmetricHull * asymmetricHull ) {
  GEOM_ASSERT(asymmetricHull);

  GEOM_DISCRETIZER_CHECK_CACHE(asymmetricHull);

  const Vector3& _botPoint = asymmetricHull->getBottom();
  const Vector3& _topPoint = asymmetricHull->getTop();
  const real_t& _shapeBot = asymmetricHull->getBottomShape();
  const real_t& _shapeTop = asymmetricHull->getTopShape();
  uint32_t _slices = asymmetricHull->getSlices();
  uint32_t _stacks = asymmetricHull->getStacks();

  uint32_t _totalSlices = _slices * 4;
  uint32_t _totalStacks = _stacks * 2;

  uint32_t _ringCount = _totalStacks - 1;     // number of rings of points
  uint32_t _bot = _totalSlices * _ringCount;  // index of the lower point
  uint32_t _top = _bot + 1;                   // index of the upper point

  Point3ArrayPtr _pointList(new Point3Array(_top + 1));
  Index3ArrayPtr _indexList(new Index3Array(_ringCount * _totalSlices * 2));

  uint32_t _cur = 0;
  uint32_t _next = _ringCount;

  uint32_t _pointCount = 0;
  uint32_t _indexCount = 0;

  real_t _azStep = real_t(GEOM_TWO_PI) / _totalSlices;
  real_t _azIter = 0;

  real_t _radius1;
  real_t _radius2;
 // real_t _deltaZ;
 // real_t _zStep;
 // real_t _zIter;

  real_t _z1;
  real_t _z2;

  // Constructs quadrats by quadrats
  for (uint32_t _q = 0; _q < 4; _q++) {

    switch(_q) {
    case 0:
      _radius1 = asymmetricHull->getPosXRadius();
      _radius2 = asymmetricHull->getPosYRadius();
	  _z1 = asymmetricHull->getPosXHeight();
	  _z2 = asymmetricHull->getPosYHeight();
      // _zIter = asymmetricHull->getPosXHeight();
      // _deltaZ = asymmetricHull->getPosYHeight() - _zIter;
      break;
    case 1:
      _radius1 = asymmetricHull->getNegXRadius();
      _radius2 = asymmetricHull->getPosYRadius();
	  _z1 = asymmetricHull->getNegXHeight();
	  _z2 = asymmetricHull->getPosYHeight();
      // _zIter = asymmetricHull->getPosYHeight();
      // _deltaZ = asymmetricHull->getNegXHeight() - _zIter;
      break;
    case 2:
      _radius1 = asymmetricHull->getNegXRadius();
      _radius2 = asymmetricHull->getNegYRadius();
	  _z1 = asymmetricHull->getNegXHeight();
	  _z2 = asymmetricHull->getNegYHeight();
      // _zIter = asymmetricHull->getNegXHeight();
      // _deltaZ = asymmetricHull->getNegYHeight() - _zIter;
      break;
    default:
      _radius1 = asymmetricHull->getPosXRadius();
      _radius2 = asymmetricHull->getNegYRadius();
 	  _z1 = asymmetricHull->getPosXHeight();
	  _z2 = asymmetricHull->getNegYHeight();
      // _zIter = asymmetricHull->getNegYHeight();
      // _deltaZ = asymmetricHull->getPosXHeight() - _zIter;
    };
      // _zStep = _deltaZ / _slices;

    for (uint32_t _i = 0; _i < _slices; _i++) {

      // Computes the peripheral point
      real_t _cosAz = cos(_azIter);
      real_t _sinAz = sin(_azIter);

      // Vector3 _Pi(_cosAz * _radius1,_sinAz * _radius2,_zIter);
      Vector3 _Pi(_cosAz * _radius1,_sinAz * _radius2,_z1 * _cosAz *_cosAz + _z2 * _sinAz * _sinAz );

      // Constructs the Lower part
      _indexList->setAt(_indexCount++,Index3(_cur, _bot, _next));

      Vector3::Cylindrical _PiBot(_botPoint - _Pi);
      real_t _rStepBot = _PiBot.radius / _stacks;
      real_t _rIterBot = 0;

      for (uint32_t _j = 0; _j < _stacks - 1; _j++) {
        _rIterBot += _rStepBot;

        Vector3::Cylindrical _PijBot(_rIterBot,
                                     _PiBot.theta,
                                     _PiBot.z * pow(_rIterBot / _PiBot.radius,
                                                    _shapeBot));

        Vector3 _Pij(_botPoint - Vector3(_PijBot));

        _pointList->setAt(_pointCount++,_Pij);

        _indexList->setAt(_indexCount++,
                          Index3(_cur + _j,
                                 _next + _j,
                                 _next + _j + 1));
        _indexList->setAt(_indexCount++,
                          Index3(_cur + _j,
                                 _next + _j + 1,
                                 _cur + _j + 1));
      };

      // Adds the peripheral point
      _pointList->setAt(_pointCount++,_Pi);

      // Constructs the upper part
      Vector3::Cylindrical _PiTop(_topPoint - _Pi);
      real_t _rStepTop = _PiTop.radius / _stacks;
      real_t _rIterTop = _PiTop.radius;

      for (uint32_t _j2 = 0; _j2 < _stacks - 1; _j2++) {
        _rIterTop -= _rStepTop;

        Vector3::Cylindrical _PijTop(_rIterTop,
                                     _PiTop.theta,
                                     _PiTop.z * pow(_rIterTop / _PiTop.radius,
                                                    _shapeTop));
        Vector3 _Pij(_topPoint - Vector3(_PijTop));

        _pointList->setAt(_pointCount++,_Pij);

        _indexList->setAt(_indexCount++,
                          Index3(_cur + _stacks - 1 + _j2,
                                 _next + _stacks - 1 + _j2,
                                 _next + _stacks +_j2));
        _indexList->setAt(_indexCount++,
                          Index3(_cur + _stacks - 1 + _j2,
                                 _next + _stacks + _j2,
                                 _cur + _stacks + _j2));
      };

      _indexList->setAt(_indexCount++,
                        Index3(_cur + _ringCount - 1,
                               _next + _ringCount - 1,
                               _top));

      _cur = _next;
      _next = (_next + _ringCount) % _bot;
      _azIter += _azStep;
     //  _zIter += _zStep;
    };

  };

  _pointList->setAt(_pointCount++,_botPoint);
  _pointList->setAt(_pointCount++,_topPoint);

  GEOM_ASSERT(_pointCount == _pointList->getSize());
  GEOM_ASSERT(_indexCount == _indexList->getSize());

  PolylinePtr _skeleton(new Polyline(_botPoint,_topPoint));

  __discretization = ExplicitModelPtr(new TriangleSet(_pointList,
                                           _indexList,
										   true,
                                           true, // CCW
                                           true,
                                           _skeleton));

  GEOM_DISCRETIZER_UPDATE_CACHE(asymmetricHull);
  return true;
}


/* ----------------------------------------------------------------------- */


bool Discretizer::process( AxisRotated * axisRotated ) {
  GEOM_ASSERT(axisRotated);
  GEOM_DISCRETIZER_TRANSFORM(axisRotated );
  return true;
}


/* ----------------------------------------------------------------------- */


bool Discretizer::process( BezierCurve * bezierCurve ) {
  GEOM_ASSERT(bezierCurve);

  GEOM_DISCRETIZER_CHECK_CACHE(bezierCurve);

  real_t _start = 0;
  uint32_t _size = bezierCurve->getStride();
  real_t _step = real_t(1.0) / (real_t)_size;
  Point3ArrayPtr _pointList(new Point3Array(_size + 1));

  for (uint32_t _i = 0; _i <= _size; _i++) {
    _pointList->setAt(_i,bezierCurve->getPointAt(_start));
    _start += _step;
  };

  __discretization = ExplicitModelPtr(new Polyline(_pointList));

  GEOM_DISCRETIZER_UPDATE_CACHE(bezierCurve);
  return true;
}


/* ----------------------------------------------------------------------- */


bool Discretizer::process( BezierPatch * bezierPatch ) {
  GEOM_ASSERT(bezierPatch);

  GEOM_DISCRETIZER_CHECK_CACHE_WITH_TEX(bezierPatch);

  const uint32_t _uStride = bezierPatch->getUStride();
  const uint32_t _vStride = bezierPatch->getVStride();

  const real_t _uStride1 = bezierPatch->getUStride() - real_t(1);
  const real_t _vStride1 = bezierPatch->getVStride() - real_t(1);

  Point3ArrayPtr _pointList(new Point3Array(_uStride * _vStride));
  Index4ArrayPtr _indexList(new Index4Array( (_uStride - 1) * (_vStride - 1)));

  uint32_t _cur = 0;

  uint32_t _pointCount = 0;
  uint32_t _indexCount = 0;

  for ( real_t _u = 0 ; _u < _uStride1 ; _u ++){

    for (real_t _v = 0; _v < _vStride1; _v ++) {

      _pointList->setAt(_pointCount++,
                        bezierPatch->getPointAt((_u/_uStride1),
                                                (_v/_vStride1)));

      _indexList->setAt(_indexCount++,Index4(_cur,
                                             _cur + 1,
                                             _cur + _uStride + 1,
                                             _cur + _uStride));

      _cur++;

    };

    _pointList->setAt(_pointCount++,
                      bezierPatch->getPointAt((_u/_uStride1),1.0));

    _cur++;

  };

  for (real_t _v = 0; _v < _vStride1; _v ++)
    _pointList->setAt(_pointCount++,
                      bezierPatch->getPointAt(1.0,(_v/_vStride1)));

  _pointList->setAt(_pointCount,bezierPatch->getPointAt(1.0,1.0));

  PolylinePtr _skeleton(new Polyline(Vector3(0,0,0),
                                     Vector3(0,0,0)));

  QuadSet * q  = new QuadSet(_pointList,
                              _indexList,
							  true,
                              bezierPatch->getCCW(), // CCW
                              false,
                              _skeleton);

  if(__computeTexCoord)q->getTexCoordList() = gridTexCoord(_pointList,_uStride,_vStride);

  __discretization = ExplicitModelPtr(q);
  

  GEOM_DISCRETIZER_UPDATE_CACHE(bezierPatch);
  return true;
}


/* ----------------------------------------------------------------------- */


bool Discretizer::process( Box * box ) {
  GEOM_ASSERT(box);

  GEOM_DISCRETIZER_CHECK_CACHE(box);

  const Vector3& _size = box->getSize();

  Point3ArrayPtr _pointList(new Point3Array(8));
  Index4ArrayPtr _indexList(new Index4Array(6));
  Point3ArrayPtr _normalList(new Point3Array(6));

  _pointList->setAt(0,Vector3( _size.x(),-_size.y(),-_size.z()));
  _pointList->setAt(1,Vector3(-_size.x(),-_size.y(),-_size.z()));
  _pointList->setAt(2,Vector3(-_size.x(), _size.y(),-_size.z()));
  _pointList->setAt(3,Vector3( _size.x(), _size.y(),-_size.z()));
  _pointList->setAt(4,Vector3( _size.x(),-_size.y(), _size.z()));
  _pointList->setAt(5,Vector3(-_size.x(),-_size.y(), _size.z()));
  _pointList->setAt(6,Vector3(-_size.x(), _size.y(), _size.z()));
  _pointList->setAt(7,Vector3( _size.x(), _size.y(), _size.z()));

  _indexList->setAt(0,Index4(0,1,2,3));
  _indexList->setAt(1,Index4(0,3,7,4));
  _indexList->setAt(2,Index4(1,0,4,5));
  _indexList->setAt(3,Index4(2,1,5,6));
  _indexList->setAt(4,Index4(3,2,6,7));
  _indexList->setAt(5,Index4(4,7,6,5));


  PolylinePtr _skeleton(new Polyline(Vector3(0,0,-_size.z()),
                                     Vector3(0,0,_size.z())));

  __discretization = ExplicitModelPtr(new QuadSet(_pointList,
                                                  _indexList,
												  false,
                                                  true, // CCW
                                                  true, // solid
                                                  _skeleton));

  GEOM_DISCRETIZER_UPDATE_CACHE(box);
  return true;
}


/* ----------------------------------------------------------------------- */


bool Discretizer::process( Cone * cone ) {
  GEOM_ASSERT(cone);

  GEOM_DISCRETIZER_CHECK_CACHE(cone);

  real_t _radius = cone->getRadius();
  real_t _height = cone->getHeight();
  bool _solid = cone->getSolid();
  uint32_t _slices = cone->getSlices();

  uint32_t _offset = (_solid ? 1 : 0);

  Point3ArrayPtr _pointList(new Point3Array(_slices + 1 + _offset));
  Index3ArrayPtr _indexList(new Index3Array(_slices * (1 + _offset)));

  uint32_t _cur = 0;
  uint32_t _next = 1;
  uint32_t _base = _slices;
  uint32_t _top = _base + 1;

  uint32_t _pointsCount = 0;
  uint32_t _facesCount = 0;
  real_t _angleStep = real_t(GEOM_TWO_PI) / _slices;

  _pointList->setAt(_top,Vector3(0,0,_height));

  for (uint32_t _i = 0; _i < _slices; _i++) {
    real_t _x = cos(_i * _angleStep) * _radius;
    real_t _y = sin(_i * _angleStep) * _radius;

    _pointList->setAt(_pointsCount++,Vector3(_x,_y,0));

    _indexList->setAt(_facesCount++,Index3(_cur,_next,_top));

    if (_solid)
      _indexList->setAt(_facesCount++,Index3(_cur,_base,_next));

    _cur = _next;
    _next = (_next + 1 ) % _slices;
  }

  PolylinePtr _skeleton(new Polyline(Vector3(Vector3::ORIGIN),
                                     Vector3(0,0,_height)));

  __discretization = ExplicitModelPtr(new TriangleSet(_pointList,
                                                      _indexList,
													  true,
                                                      true, // CCW
                                                      _solid,
                                                      _skeleton));

  GEOM_DISCRETIZER_UPDATE_CACHE(cone);
  return true;
}


/* ----------------------------------------------------------------------- */


bool Discretizer::process( Cylinder * cylinder ) {
  GEOM_ASSERT(cylinder);

  GEOM_DISCRETIZER_CHECK_CACHE(cylinder);

  real_t _radius = cylinder->getRadius();
  real_t _height = cylinder->getHeight();
  bool _solid = cylinder->getSolid();
  uint32_t _slices = cylinder->getSlices();

  uint32_t _offset = (_solid ? 2 : 0);

  Point3ArrayPtr _pointList(new Point3Array((_slices * 2) + _offset));
  IndexArrayPtr _indexList;
  Index4ArrayPtr _index4List;
  if(_solid)
      _indexList= IndexArrayPtr(new IndexArray(_slices * 3 ));
  else
      _index4List= Index4ArrayPtr(new Index4Array(_slices ));


  uint32_t _cur = 0;
  uint32_t _next = 2;
  uint32_t _base = 2 * _slices;
  uint32_t _top = _base + 1;

  uint32_t _pointsCount = 0;
  uint32_t _facesCount = 0;
  real_t _angleStep = GEOM_TWO_PI / _slices;

  if (_solid)
    _pointList->setAt(_top,Vector3(0,0,_height));

  for (uint32_t _i = 0; _i < _slices; _i++) {
    real_t _x = cos(_i * _angleStep) * _radius;
    real_t _y = sin(_i * _angleStep) * _radius;
    _pointList->setAt(_pointsCount++,Vector3(_x,_y,0));
    _pointList->setAt(_pointsCount++,Vector3(_x,_y,_height));

    if (_solid) {
        Index _index4(4);
        _index4.setAt(0,_cur);_index4.setAt(1,_next);_index4.setAt(2,_next+1);_index4.setAt(3,_cur+1);
        _indexList->setAt(_facesCount++,_index4);
        Index _index3(3);
        _index3.setAt(0,_cur + 1);_index3.setAt(1,_next + 1);_index3.setAt(2,_top);
        _indexList->setAt(_facesCount++,_index3);
        Index _index3b(3);
        _index3b.setAt(0,_cur);_index3b.setAt(1,_base);_index3b.setAt(2,_next);
        _indexList->setAt(_facesCount++,_index3b);
    }
    else {
        _index4List->setAt(_facesCount++,Index4(_cur,_next ,_next+1,_cur + 1));
    }


    _cur = _next;
    _next = (_next + 2 ) % (2 * _slices);
  }

  PolylinePtr _skeleton(new Polyline(Vector3(0,0,0),
                                     Vector3(0,0,_height)));


  if (_solid)
      __discretization = ExplicitModelPtr(new FaceSet(_pointList,
                                                      _indexList,
													  true,
                                                      true, // CCW
                                                      _solid,
                                                      _skeleton));
  else
      __discretization = ExplicitModelPtr(new QuadSet(_pointList,
                                                      _index4List,
													  true,
                                                      true, // CCW
                                                      _solid,
                                                      _skeleton));

//  Printer _p(cerr,cerr,cerr);
//  __discretization->apply(_p);
  GEOM_DISCRETIZER_UPDATE_CACHE(cylinder);
  return true;
}


/* ----------------------------------------------------------------------- */



bool Discretizer::process( ElevationGrid * elevationGrid ) {
  GEOM_ASSERT(elevationGrid);

  GEOM_DISCRETIZER_CHECK_CACHE_WITH_TEX(elevationGrid);

  const uint32_t& _xDim = elevationGrid->getXDim();
  const uint32_t& _yDim = elevationGrid->getYDim();

  uint32_t _colCount = _yDim - 1;
  uint32_t _rowCount = _xDim - 1;

  Point3ArrayPtr _pointList(new Point3Array(_xDim * _yDim));
  Index3ArrayPtr _indexList(new Index3Array(_rowCount * _colCount * 2));
  uint32_t _cur = 0;
  uint32_t _next = 1;

  uint32_t _pointCount = 0;
  uint32_t _indexCount = 0;


  for (uint32_t _j = 0; _j < _yDim; _j++) {
    for (uint32_t _i = 0; _i < _xDim; _i++) {

      _pointList->setAt(_pointCount++,elevationGrid->getPointAt(_i,_j));
      if ((_i < _rowCount) && (_j < _colCount)) {
        _indexList->setAt(_indexCount++,
                          Index3(_cur,_next,_cur + _xDim));
        _indexList->setAt(_indexCount++,
                          Index3(_next,_next + _xDim,_cur + _xDim));
      };

      _cur = _next;
      _next = _next + 1;
    };

  };

  PolylinePtr _skeleton(new Polyline(Vector3(0,0,0),
                                     Vector3(0,0,0)));

  TriangleSet * t = new TriangleSet(_pointList, _indexList, true, elevationGrid->getCCW(), // CCW
                                    false, _skeleton);
  if(__computeTexCoord)t->getTexCoordList() = gridTexCoord(_pointList,_yDim,_xDim);

  __discretization = ExplicitModelPtr(t);
  GEOM_DISCRETIZER_UPDATE_CACHE(elevationGrid);
  return true;
}


/* ----------------------------------------------------------------------- */


bool Discretizer::process( EulerRotated * eulerRotated ) {
  GEOM_ASSERT(eulerRotated);
  GEOM_DISCRETIZER_TRANSFORM(eulerRotated );
  return true;
}


/* ----------------------------------------------------------------------- */


bool Discretizer::process( ExtrudedHull * extrudedHull ) {
  GEOM_ASSERT(extrudedHull);

  GEOM_DISCRETIZER_CHECK_CACHE(extrudedHull);

  const Curve2DPtr _horcurve = extrudedHull->getHorizontal();
  const Curve2DPtr _vercurve = extrudedHull->getVertical();



  if(!_horcurve->apply(*this)){
    /// Hack for bug with tesselator.
    Discretizer second;
    if(!_horcurve->apply(second)){
      cerr << "Warning ! could not perform discretization on horizontal profile of " << extrudedHull->getName() << endl;
      return false;
    }
    else {
      cerr << "Warning ! use a hack in discretizer for " << extrudedHull->getName() << endl;
      __discretization = second.getDiscretization();
    }
  }
  const Point3ArrayPtr _horizontal = __discretization->getPointList();
  if(!_horizontal)return false;

  if(!_vercurve->apply(*this)){
    /// Hack for bug with tesselator.
    Discretizer second;
    if(!_vercurve->apply(second)){
      cerr << "Warning ! could not perform discretization on vertical profile of " << extrudedHull->getName() << endl;
      return false;
    }
    else {
      __discretization = second.getDiscretization();
    }
  }
  const Point3ArrayPtr _vertical = __discretization->getPointList();
  if(!_vertical)return false;

  pair<Point3Array::const_iterator,Point3Array::const_iterator> _xMinAndMax = _horizontal->getXMinAndMax();
  pair<Point3Array::const_iterator,Point3Array::const_iterator> _yMinAndMax = _horizontal->getYMinAndMax();
  real_t _width = fabs(_xMinAndMax.second->x() - _xMinAndMax.first->x());

  // Vector3 _center = _horizontal->getCenter();
  real_t _xcenter = (_xMinAndMax.second->x() + _xMinAndMax.first->x())/2;
  real_t _ycenter = (_yMinAndMax.second->y() + _yMinAndMax.first->y())/2;

  uint32_t _hSize = _horizontal->getSize();
  uint32_t _vSize = _vertical->getSize();


  // Computes the position of the top
  Point3Array::const_iterator _begin = _vertical->getBegin();
  pair<Point3Array::const_iterator,Point3Array::const_iterator>
    _minAndMax = _vertical->getYMinAndMax();
  uint32_t _ndxBot = distance(_begin,_minAndMax.first);
  uint32_t _ndxTop = distance(_begin,_minAndMax.second);

  vector<uint32_t> _ndx1;
  real_t _len1 = 0;
  uint32_t _iNdx = _ndxBot;

  while (_iNdx != _ndxTop) {
    _ndx1.push_back(_iNdx);
    uint32_t _jNdx = (_iNdx + 1) % _vSize;
    _len1 += norm(_vertical->getAt(_iNdx) - _vertical->getAt(_jNdx));
    _iNdx = _jNdx;
  };
  _ndx1.push_back(_ndxTop);

  vector<uint32_t> _ndx2;
  real_t _len2 = 0;
  _iNdx = _ndxBot;

  while (_iNdx != _ndxTop) {
    _ndx2.push_back(_iNdx);
    uint32_t _jNdx = _iNdx == 0 ? _vSize - 1 : _iNdx - 1;
    _len2 += norm(_vertical->getAt(_iNdx) - _vertical->getAt(_jNdx));
    _iNdx = _jNdx;
  };

  _ndx2.push_back(_ndxTop);

  uint32_t _stacks1 = _ndx1.size() - 1;
  uint32_t _stacks2 = _ndx2.size() - 1;
  uint32_t _stacks = std::max(_stacks1,_stacks2);
//  uint32_t _stacks = _stacks1 + _stacks2;

  uint32_t _ring = _stacks - 1;
  uint32_t _ringBySize = _ring * _hSize;

  uint32_t _bot = _ringBySize;
  uint32_t _top = _bot + 1;
  uint32_t _cur = 0;

  Point3ArrayPtr _pointList(new Point3Array(_ringBySize + 2));
  Index3ArrayPtr _indexList(new Index3Array(_ringBySize * 2));

  uint32_t _pointCount = 0;
  uint32_t _indexCount = 0;

  Vector3 _p1, _p2;

  real_t _dtSeg1 = norm(_vertical->getAt(_ndx1[0]) -
                      _vertical->getAt(_ndx1[1])) / _len1;
  real_t _dtSeg2 = norm(_vertical->getAt(_ndx2[0]) -
                      _vertical->getAt(_ndx2[1])) / _len2;

  real_t _dt1 = _dtSeg1;
  real_t _dt2 = _dtSeg2;

  uint32_t _i1 = 0;
  uint32_t _i2 = 0;
  for (uint32_t _i = 0; _i < _hSize; _i++) {
    uint32_t _j = (_i + 1) % _hSize;
    _indexList->setAt(_indexCount++,Index3(_cur + _i, _bot, _cur + _j));
  };

  for (uint32_t _iStacks = 1; _iStacks < _stacks; _iStacks++) {

    real_t _t = (real_t)_iStacks / (real_t)_stacks;


    // Computes p1

    if (_stacks1 != _stacks) {
      while (_dt1 < _t) {
        _i1++;
        _dtSeg1 = norm(_vertical->getAt(_ndx1[_i1]) -
                       _vertical->getAt(_ndx1[_i1 + 1])) / _len1;
        if (_dtSeg1 > GEOM_TOLERANCE) _dt1 += _dtSeg1;
      };
      GEOM_ASSERT(_dtSeg1 > GEOM_TOLERANCE);
      real_t _alpha1 = (_dt1 - _t) / _dtSeg1;
      _p1 = (_vertical->getAt(_ndx1[_i1 + 1]) * (1 - _alpha1) +
             _vertical->getAt(_ndx1[_i1]) * _alpha1);
    }
    else
        _p1 = _vertical->getAt(_ndx1[++_i1]);
    //_p1.print(cout) << endl;

    // Computes p2
    if (_stacks2 != _stacks) {
      while (_dt2 < _t) {
        _i2++;
        _dtSeg2 = norm(_vertical->getAt(_ndx2[_i2]) -
                       _vertical->getAt(_ndx2[_i2 + 1])) / _len2;
        if (_dtSeg2 > GEOM_TOLERANCE) _dt2 += _dtSeg2;
      };
      GEOM_ASSERT(_dtSeg1 > GEOM_TOLERANCE);
      real_t _alpha2 = (_dt2 - _t) / _dtSeg2;
      _p2 = (_vertical->getAt(_ndx2[_i2 + 1]) * (1 - _alpha2) +
             _vertical->getAt(_ndx2[_i2]) * _alpha2);
    }
    else
      _p2 = _vertical->getAt(_ndx2[++_i2]);
    //_p2.print(cout) << endl;

    // To map the horizontal profile we used a transformation  composed of:
    // - a rotation about (0,1,0) of the elevation of [P1P2]
    // - a scaling of |P1P2| / width of the horizontal profile
    // - a translation of P1
    Vector3 _p12(_p2 - _p1);
    if(_p12.x() < GEOM_EPSILON) _p12 = -_p12;
    real_t _norm = norm(_p12);
    GEOM_ASSERT(_norm > GEOM_TOLERANCE);
    real_t _cosA = + _p12.x() / _norm;
    real_t _sinA = + _p12.y() / _norm;
    real_t _sf = _norm / _width;

    Vector2 _v( (_p1.x() + _p2.x()) / 2 ,
                (_p1.y() + _p2.y()) / 2 );

    for (uint32_t _hPoint = 0; _hPoint < _hSize; _hPoint++) {

      const Vector2 _p = Vector2(_horizontal->getAt(_hPoint).x(),_horizontal->getAt(_hPoint).y());

      _pointList->setAt(_pointCount++,
                        Vector3(_cosA * _sf * (_p.x()- _xcenter) + _v.x(),
                                _sf * (_p.y()-_ycenter) + _ycenter,
                                _sinA * _p.x() * _sf + _v.y()));

      uint32_t _hNextPoint = (_hPoint + 1) % _hSize;
      if (_iStacks != _stacks - 1) {
        _indexList->setAt(_indexCount++,
                          Index3(_cur + _hPoint,
                                 _cur + _hNextPoint,
                                 _cur + _hNextPoint + _hSize));
        _indexList->setAt(_indexCount++,
                          Index3(_cur + _hPoint,
                                 _cur + _hNextPoint + _hSize,
                                 _cur + _hPoint + _hSize));
      }
      else
        _indexList->setAt(_indexCount++,
                          Index3(_cur + _hPoint,
                                 _cur + _hNextPoint,
                                 _top));

    };

    _cur += _hSize;

  };

  _pointList->setAt(_pointCount++,Vector3(_vertical->getAt(_ndxBot).x(),
                                          _ycenter,
                                          _vertical->getAt(_ndxBot).y()));

  _pointList->setAt(_pointCount++,Vector3(_vertical->getAt(_ndxTop).x(),
                                          _ycenter,
                                          _vertical->getAt(_ndxTop).y()));

  GEOM_ASSERT(_pointCount == _pointList->getSize());
  GEOM_ASSERT(_indexCount == _indexList->getSize());

  PolylinePtr _skeleton(new Polyline(Vector3::ORIGIN,
                                     Vector3::ORIGIN));

  __discretization = ExplicitModelPtr(new TriangleSet(_pointList,
                                                      _indexList,
													  true,
                                                      extrudedHull->getCCW(), // CCW
                                                      true, // Solid
                                                      _skeleton));
  GEOM_DISCRETIZER_UPDATE_CACHE(extrudedHull);
  return true;
}


/* ----------------------------------------------------------------------- */


bool Discretizer::process( Frustum * frustum ) {
  GEOM_ASSERT(frustum);

  GEOM_DISCRETIZER_CHECK_CACHE(frustum);

  real_t _radius = frustum->getRadius();
  real_t _height = frustum->getHeight();
  real_t _taper = frustum->getTaper();
  bool _solid = frustum->getSolid();
  uint32_t _slices = frustum->getSlices();

  uint32_t _offset = (_solid ? 2 : 0);

  Point3ArrayPtr _pointList(new Point3Array((_slices * 2) + _offset));
  IndexArrayPtr _indexList;
  Index4ArrayPtr _index4List;
  if(_solid)
      _indexList= IndexArrayPtr(new IndexArray(_slices * 3 ));
  else
      _index4List= Index4ArrayPtr(new Index4Array(_slices ));



  uint32_t _cur = 0;
  uint32_t _next = 2;
  uint32_t _base = 2 * _slices;
  uint32_t _top = _base + 1;

  uint32_t _pointsCount = 0;
  uint32_t _facesCount = 0;
  real_t _angleStep = GEOM_TWO_PI / _slices;

  if (_solid)
    _pointList->setAt(_top,Vector3(0,0,_height));

  for (uint32_t _i = 0; _i < _slices; _i++) {
    real_t _x = cos(_i * _angleStep) * _radius;
    real_t _y = sin(_i * _angleStep) * _radius;

    _pointList->setAt(_pointsCount++,Vector3(_x,_y,0));
    _pointList->setAt(_pointsCount++,Vector3(_x * _taper,
                                             _y * _taper,
                                             _height));

    if (_solid) {
        Index _index4(4);
        _index4.setAt(0,_cur);_index4.setAt(1,_next);_index4.setAt(2,_next+1);_index4.setAt(3,_cur+1);
        _indexList->setAt(_facesCount++,_index4);
        Index _index3(3);
        _index3.setAt(0,_cur + 1);_index3.setAt(1,_next + 1);_index3.setAt(2,_top);
        _indexList->setAt(_facesCount++,_index3);
        Index _index3b(3);
        _index3b.setAt(0,_cur);_index3b.setAt(1,_base);_index3b.setAt(2,_next);
        _indexList->setAt(_facesCount++,_index3b);
    }
    else {
        _index4List->setAt(_facesCount++,Index4(_cur,_next ,_next+1,_cur + 1));
    }

    _cur = _next;
    _next = (_next + 2 ) % (2 * _slices);
  }

  PolylinePtr _skeleton(new Polyline(Vector3(0,0,0),
                                     Vector3(0,0,_height)));

  if (_solid)
      __discretization = ExplicitModelPtr(new FaceSet(_pointList,
                                                      _indexList,
													  true,
                                                      true, // CCW
                                                      _solid,
                                                      _skeleton));
  else
      __discretization = ExplicitModelPtr(new QuadSet(_pointList,
                                                      _index4List,
													  true,
                                                      true, // CCW
                                                      _solid,
                                                      _skeleton));

  GEOM_DISCRETIZER_UPDATE_CACHE(frustum);
  return true;
}


/* ----------------------------------------------------------------------- */


bool Discretizer::process( Extrusion * extrusion ){
    GEOM_ASSERT(extrusion);
    GEOM_DISCRETIZER_CHECK_CACHE_WITH_TEX(extrusion);
    if(!(extrusion->getCrossSection()->apply(*this))){
        cerr << "Warning ! could not perform discretization on Cross Section of " << extrusion->getName() << endl;
        __discretization = ExplicitModelPtr();
        return false;
    }
    ExplicitModelPtr _explicitCrossSection(__discretization);
    if(!_explicitCrossSection){
        cerr << "Warning ! could not perform discretization on Cross Section of " << extrusion->getName() << endl;
        GEOM_ASSERT(_explicitCrossSection);
        __discretization = ExplicitModelPtr();
        return false;
    }

    Point3ArrayPtr _crossPoints = _explicitCrossSection->getPointList();
	bool closed = false;
	if(!(norm(_crossPoints->getAt(0) - _crossPoints->getAt(_crossPoints->getSize()-1)) > GEOM_EPSILON)){
	  _crossPoints = Point3ArrayPtr(new Point3Array(_crossPoints->getBegin(),_crossPoints->getEnd() -1));
	  closed = true;
	}

    uint32_t _nbPoints = _crossPoints->getSize();

    LineicModelPtr _axis = extrusion->getAxis();

    ProfileTransformationPtr _profileTransf = extrusion->getProfileTransformation();
    bool _useTransf = true;
    if(!_profileTransf)_useTransf = false;

    real_t _start = _axis->getFirstKnot();
    uint32_t _size =  _axis->getStride();
    real_t _step =  (_axis->getLastKnot()-_start) / (real_t) _size;
    real_t _starttransf = 0;
    real_t _steptransf = 0;
    if(_useTransf){
        _starttransf = _profileTransf->getUMin();
        _steptransf = (_profileTransf->getUMax()-_starttransf) /  _size;
    }

    Point3ArrayPtr _pointList(new Point3Array(((_size+1)*(_nbPoints))));
    Point2ArrayPtr _texList(0);
	if(__computeTexCoord)_texList = Point2ArrayPtr(new Point2Array(((_size+1)*(_nbPoints))));
    Index4ArrayPtr _indexList(new Index4Array((_size)*(_nbPoints-(closed?0:1))));
    uint32_t _j = 0;
    uint32_t _k = 0;

    Vector3 _oldBinormal;
    Vector3 _normal;( _axis->getNormalAt(_start) );
    _normal= ( _axis->getNormalAt(_start) );
    if( normSquared(_normal) < GEOM_EPSILON )
      {
      // normal==0 : curve is locally like a line
      Vector3 tg( _axis->getTangentAt(_start) );
      Vector3 U=
        ( tg.x() < tg.y() ) ? ( ( tg.z() < tg.x() ) ? Vector3::OZ
                                                    : Vector3::OX )
                            : ( ( tg.z() < tg.y() ) ? Vector3::OZ
                                                    : Vector3::OY );

      _normal= cross(tg,U);
      }

    for (uint32_t _i = 0; _i < _size; _i++) {
        Vector3 _center = _axis->getPointAt(_start);
        Vector3 _velocity = _axis->getTangentAt(_start);
        if(_i!=0)
            _normal = cross(_oldBinormal,_velocity);
        else _normal = _axis->getNormalAt(_start);
        _velocity.normalize();
        _normal.normalize();
        Vector3 _binormal = cross(_velocity,_normal);
        _binormal.normalize();
        _oldBinormal = _binormal;

        Matrix3 _frame(_normal,_binormal,_velocity);
        OrthonormalBasis3D _transf(_frame);
        Point3ArrayPtr _newPoint;
        if(_useTransf){
            Transformation2DPtr _transf2D =  (*_profileTransf)(_starttransf);
            _newPoint = _transf2D->transform(_crossPoints);
            _newPoint =  _transf.transform(_newPoint);
        }
        else _newPoint =  _transf.transform(_crossPoints);
        if(closed ){
           _indexList->setAt(_k,Index4(_j+_nbPoints-1,_j,_j+_nbPoints,_j+2*_nbPoints-1));_k++;
		}
		float _idPoint = 0;
        for(Point3Array::iterator _it = _newPoint->getBegin();
            _it != _newPoint->getEnd();
            ++_it,++_idPoint,++_j){
            _pointList->setAt(_j,((*_it)+_center));
			if(__computeTexCoord)
				_texList->setAt(_j,Vector2(_start,_idPoint/(_nbPoints-1)));
            if((_j+1)%(_nbPoints)!=0){
                _indexList->setAt(_k,Index4(_j,_j+1,_j+_nbPoints+1,_j+_nbPoints));_k++;
            }
        }
        _start += _step;
        _starttransf += _steptransf;
    };
    _start= _axis->getLastKnot();
    if(_useTransf){
        _starttransf = _profileTransf->getUMax();
    }
    Vector3 _velocity = _axis->getTangentAt(_start);
    _normal = cross(_oldBinormal,_velocity);
    _velocity.normalize();
    _normal.normalize();
    Vector3 _binormal = cross(_velocity,_normal);
    Matrix3 _frame(_normal,_binormal,_velocity);
    OrthonormalBasis3D _transf(_frame);
    Vector3 _center = _axis->getPointAt(_start);
    Point3ArrayPtr _newPoint;
    if(_useTransf){
        Transformation2DPtr _transf2D =  (*_profileTransf)(_starttransf);
        _newPoint = _transf2D->transform(_crossPoints);
        _newPoint =  _transf.transform(_newPoint);
    }
    else _newPoint =  _transf.transform(_crossPoints);
	float _idPoint = 0;
    for(Point3Array::iterator _it = _newPoint->getBegin();
        _it != _newPoint->getEnd();
        ++_it,++_idPoint,++_j){
        _pointList->setAt(_j,((*_it)+_center));
		if(__computeTexCoord)_texList->setAt(_j,Vector2(_start,_idPoint/(_nbPoints-1)));
    }
    PolylinePtr _skeleton(new Polyline(Vector3(0,0,0),
                                     Vector3(0,0,0)));

	Mesh * m;
    if(extrusion->getSolid()){
        IndexArrayPtr _indexList2(new IndexArray(2));
        Index a(_nbPoints);
        Index b(_nbPoints);
        for(uint32_t _p=0; _p < _nbPoints; _p++){
            a.setAt(_p,_p);
            b.setAt(_p,(_size*_nbPoints)+_p);
        }
        _indexList2->setAt(0,a);
        _indexList2->setAt(1,b);
        Index3ArrayPtr _cap = _indexList2->triangulate();
        _indexList2 = IndexArrayPtr(new IndexArray(_cap->getSize()+_indexList->getSize()));
        uint32_t _f =0;
        for(Index3Array::iterator _it2 = _cap->getBegin(); _it2 != _cap->getEnd() ; _it2++){
            _indexList2->setAt(_f,*_it2);_f++;
        }
        for(Index4Array::iterator _it3 = _indexList->getBegin(); _it3 != _indexList->getEnd() ; _it3++){
            _indexList2->setAt(_f,*_it3);_f++;
        }
		m = new FaceSet(_pointList,_indexList2,true,extrusion->getCCW(),true,_skeleton);
    }
    else {
		m = new QuadSet(_pointList,_indexList,true,extrusion->getCCW(),false,_skeleton);
    }
	m->getTexCoordList() = _texList;

	__discretization = ExplicitModelPtr(m);

    GEOM_DISCRETIZER_UPDATE_CACHE(extrusion);
    return true;
}

/* ----------------------------------------------------------------------- */

bool Discretizer::process( Group * group )  {
  GEOM_ASSERT(group);
  GEOM_DISCRETIZER_CHECK_CACHE(group);
  const GeometryArrayPtr& _geometryList = group->getGeometryList();
  (*(_geometryList->getBegin()))->apply(*this);
  if(!__discretization)
  {
    GEOM_DISCRETIZER_UPDATE_CACHE(group);
    return false;
  }
  ExplicitModelPtr basegeom;
  if (__discretization  == _geometryList->getBegin()->toPtr())
	  basegeom = ExplicitModelPtr::Cast(__discretization->copy());
  else basegeom = __discretization;
  Merge fusion(*this,basegeom);

  GeometryPtr geom2;
  for (GeometryArray::const_iterator _i = _geometryList->getBegin()+1;
       _i != _geometryList->getEnd();
       _i++) {
      geom2 = *_i;
      if(!fusion.apply(geom2)){
          __discretization = ExplicitModelPtr();
          GEOM_DISCRETIZER_UPDATE_CACHE(group);
          return false;
      }
  }
  __discretization = fusion.getModel();
  GEOM_DISCRETIZER_UPDATE_CACHE(group);
  return true;

}


/* ----------------------------------------------------------------------- */

bool Discretizer::process( IFS * ifs )  {
  GEOM_ASSERT(ifs);
  GEOM_TRACE("IFS->Discretizer");
//todo OK

  GEOM_DISCRETIZER_CHECK_CACHE( ifs );
  if( !ifs->getGeometry()->apply(*this) )
    {
    __discretization = ExplicitModelPtr();
    return false;
    }

#ifdef GEOM_DEBUG
  Timer chrono;
  chrono.start();
#endif

  ITPtr transfos;
  transfos.cast( ifs->getTransformation() );
  GEOM_ASSERT(transfos);
  const Matrix4ArrayPtr& matrixList= transfos->getAllTransfo();
  GEOM_ASSERT(matrixList);

#ifdef GEOM_DEBUG
  chrono.stop();
  cout << "Compute transfo ifs in " << chrono.elapsedTime() << " sec. " << endl;
  chrono.start();
#endif

  uint32_t size= matrixList->getSize();

  Matrix4Array::const_iterator matrix= matrixList->getBegin();
  Transform4Ptr t(new Transform4(*matrix));

  ExplicitModelPtr bigD = __discretization->transform(Transformation3DPtr::Cast(t));

  Merge fusion(*this,bigD);
  fusion.setIsoModel( size );

  ExplicitModelPtr tmpD;
  matrix++;
  while( matrix != matrixList->getEnd() )
    {
    t->getMatrix()= *matrix;
    tmpD= __discretization->transform(Transformation3DPtr::Cast(t));

    if(! fusion.apply(tmpD))
      {
      __discretization = ExplicitModelPtr();
      return false;
      }
    matrix++;
    }

  __discretization= fusion.getModel();

#ifdef GEOM_DEBUG
  chrono.stop();
  cout << "Fusion ifs in " << chrono.elapsedTime() << " sec. " << endl;
#endif

  GEOM_DISCRETIZER_UPDATE_CACHE(ifs);

  return true;
}


/* ----------------------------------------------------------------------- */


bool Discretizer::process( NurbsCurve * nurbsCurve ) {
  GEOM_ASSERT(nurbsCurve);

  GEOM_DISCRETIZER_CHECK_CACHE( nurbsCurve );

  real_t _start = nurbsCurve->getFirstKnot();
  uint32_t _size = nurbsCurve->getStride();
  real_t _step =  (nurbsCurve->getLastKnot()-_start) / (real_t) _size;
  Point3ArrayPtr _pointList(new Point3Array(_size + 1));

  for (uint32_t _i = 0; _i < _size; _i++) {
    _pointList->setAt(_i,nurbsCurve->getPointAt(_start));
    _start += _step;
  };

  _pointList->setAt(_size, nurbsCurve->getPointAt(nurbsCurve->getLastKnot()));

  __discretization = ExplicitModelPtr(new Polyline(_pointList));

  GEOM_DISCRETIZER_UPDATE_CACHE(nurbsCurve);
  return true;
}

/* ----------------------------------------------------------------------- */

bool Discretizer::process( NurbsPatch * nurbsPatch ) {
  GEOM_ASSERT(nurbsPatch);

  GEOM_DISCRETIZER_CHECK_CACHE_WITH_TEX(nurbsPatch);

  const uint32_t _uStride = nurbsPatch->getUStride();
  const uint32_t _vStride = nurbsPatch->getVStride();

  const real_t _uStride1 = nurbsPatch->getUStride() - 1;
  const real_t _vStride1 = nurbsPatch->getVStride() - 1;


  Point3ArrayPtr _pointList(new Point3Array(_uStride * _vStride));
  Index4ArrayPtr _indexList(new Index4Array( (_uStride - 1) * (_vStride - 1)));

  uint32_t _cur = 0;

  uint32_t _pointCount = 0;
  uint32_t _indexCount = 0;
  real_t _ufirst=nurbsPatch->getFirstUKnot();
  real_t _ulast=nurbsPatch->getLastUKnot();
  real_t _uinter=_ulast-_ufirst;
  real_t _vfirst=nurbsPatch->getFirstVKnot();
  real_t _vlast=nurbsPatch->getLastVKnot();
  real_t _vinter=_vlast-_vfirst;

  for ( real_t _u = 0 ; _u < _uStride1 ; _u ++){
    for (real_t _v = 0; _v < _vStride1; _v ++) {

      _pointList->setAt(_pointCount++,
                        nurbsPatch->getPointAt((_ufirst +
                                                (_u * _uinter) / _uStride1),
                                               (_vfirst +
                                                (_v * _vinter) / _vStride1)));

      _indexList->setAt(_indexCount++,
                        Index4(_cur,
                               _cur + 1,
                               _cur + _uStride + 1,
                               _cur + _uStride));

      _cur++;
    };

    _pointList->setAt(_pointCount++,
                      nurbsPatch->getPointAt((_u / _uStride1),1.0));

    _cur++;

  };

  for (real_t _v = 0; _v < _vStride1; _v ++){
    _pointList->setAt(_pointCount++,
                       nurbsPatch->getPointAt(_ulast,
                                               (_vfirst +
                                                (_v * _vinter) / _vStride1)));
  }

  _pointList->setAt(_pointCount,nurbsPatch->getPointAt(_ulast,_vlast));

 
  PolylinePtr _skeleton(new Polyline(Vector3(0,0,0),
                                     Vector3(0,0,0)));

  QuadSet * q = new QuadSet(_pointList,_indexList,true, nurbsPatch->getCCW(), false,_skeleton);
  if(__computeTexCoord)q->getTexCoordList() = gridTexCoord(_pointList,_uStride,_vStride);

  __discretization = ExplicitModelPtr();
  GEOM_DISCRETIZER_UPDATE_CACHE(nurbsPatch);
  return true;
}

/* ----------------------------------------------------------------------- */


bool Discretizer::process( Oriented * oriented ) {
  GEOM_ASSERT(oriented);
  GEOM_DISCRETIZER_TRANSFORM(oriented );
  return true;
}


/* ----------------------------------------------------------------------- */


bool Discretizer::process( Paraboloid * paraboloid ) {
  GEOM_ASSERT(paraboloid);

  GEOM_DISCRETIZER_CHECK_CACHE(paraboloid);

  const real_t& _radius = paraboloid->getRadius();
  const real_t& _height = paraboloid->getHeight();
  const real_t& _shape = paraboloid->getShape();
  bool _solid = paraboloid->getSolid();
  uchar_t _slices = paraboloid->getSlices();
  uchar_t _stacks = paraboloid->getStacks();

  uint32_t _stacksBySlices = _stacks * _slices;

  Point3ArrayPtr _pointList(new Point3Array(_solid ?
                                            _stacksBySlices + 2 :
                                            _stacksBySlices + 1));

  Index3ArrayPtr _indexList(new Index3Array(_solid ?
                                            _stacksBySlices * 2 :
                                            _stacksBySlices * 2 - _slices));

  real_t _angleStep = GEOM_TWO_PI / _slices;
  real_t _angleIter = 0;
  real_t _radiusStep = _radius / _stacks;
  real_t _radiusIter = _radius;

  uint32_t _cur = 0;
  uint32_t _next = _stacks;
  uint32_t _bot = _stacksBySlices;
  uint32_t _top = _stacksBySlices + (_solid ? 1 : 0);

  uint32_t _pointCount = 0;
  uint32_t _indexCount = 0;

  for (uint32_t _i = 0; _i < _slices; _i++) {
    real_t _cosAngle = cos(_angleIter);
    real_t _sinAngle = sin(_angleIter);
    real_t _x = _cosAngle * _radius;
    real_t _y = _sinAngle * _radius;
    real_t _z = 0;

    _pointList->setAt(_pointCount++,Vector3(_x,_y,_z));

    if (_solid)
      _indexList->setAt(_indexCount++,Index3(_cur,_bot,_next));

    for (uint32_t _j = 1; _j < _stacks; _j++) {

      _radiusIter -= _radiusStep;
      _x = _cosAngle * _radiusIter;
      _y = _sinAngle * _radiusIter;
      _z = _height * (1 - pow((_radiusIter / _radius),_shape));

      _pointList->setAt(_pointCount++,Vector3(_x,_y,_z));

      _indexList->setAt(_indexCount++,
                        Index3(_cur + _j, _cur + _j - 1, _next + _j - 1));
      _indexList->setAt(_indexCount++,
                        Index3(_cur + _j, _next + _j - 1, _next + _j));
    };

    _indexList->setAt(_indexCount++,
                      Index3(_cur + _stacks - 1, _next + _stacks - 1, _top));

    _angleIter += _angleStep;
    _radiusIter = _radius;
    _cur = _next;
    _next = (_next + _stacks ) % _stacksBySlices;
  };

  if (_solid)
    _pointList->setAt(_pointCount++,Vector3(Vector3::ORIGIN));

  _pointList->setAt(_pointCount++,Vector3(0,0,_height));

  GEOM_ASSERT(_pointCount == _pointList->getSize());
  GEOM_ASSERT(_indexCount == _indexList->getSize());

  PolylinePtr _skeleton(new Polyline(_pointList->getAt(_bot),
                                     _pointList->getAt(_top)));

  __discretization = ExplicitModelPtr(new TriangleSet(_pointList,
                                                      _indexList,
													  true,
                                                      true, // CCW
                                                      true,
                                                      _skeleton));
  GEOM_DISCRETIZER_UPDATE_CACHE(paraboloid);
  return true;
}


/* ----------------------------------------------------------------------- */


bool Discretizer::process( Revolution * revolution ) {
  GEOM_ASSERT(revolution);

  GEOM_DISCRETIZER_CHECK_CACHE(revolution);

  if(!revolution->getProfile()->apply(*this)){
    Discretizer second;
    if(!revolution->getProfile()->apply(second)){
      cerr << "Warning ! could not perform discretization on profile of " << revolution->getName() << endl;
      return false;
    }
    else {
      __discretization = second.getDiscretization();
    }
  }

  const Point3ArrayPtr& _curve = __discretization->getPointList();
  uint32_t _curveSize = _curve->getSize();
  uint32_t _slices = revolution->getSlices();

  Point3ArrayPtr _pointList(new Point3Array(_slices * _curveSize));
  Index3ArrayPtr _indexList(new Index3Array(_slices * 2 * (_curveSize - 1)));

  real_t _angleStep = GEOM_TWO_PI / _slices;

  uint32_t _cur = 0;
  uint32_t _next = _curveSize;

  uint32_t _pointsCount = 0;
  uint32_t _facesCount = 0;

  for (uint32_t _i = 0; _i < _slices; _i++) {

    real_t _x = cos(_i * _angleStep);
    real_t _y = sin(_i * _angleStep);

    real_t _rad = _curve->getAt(0).x();
    real_t _z = _curve->getAt(0).y();

    _pointList->setAt(_pointsCount++,Vector3(_x * _rad, _y * _rad, _z));

    for (uint32_t _j = 1; _j < _curveSize; _j++) {
      real_t _rad = _curve->getAt(_j).x();
      real_t _z = _curve->getAt(_j).y();

      _pointList->setAt(_pointsCount++,Vector3(_x * _rad, _y * _rad, _z));

      _indexList->setAt(_facesCount++,
                        Index3(_cur + _j,_cur + _j - 1,_next + _j - 1));
      _indexList->setAt(_facesCount++,
                        Index3(_cur + _j,_next + _j - 1,_next + _j));

    };

    _cur = _next;
    _next = (_next + _curveSize ) % (_curveSize * _slices);

  };

  PolylinePtr _skeleton(new Polyline(Vector3(0,0,0),
                                     Vector3(0,0,1)));

  __discretization = ExplicitModelPtr(new TriangleSet(_pointList,
                                                      _indexList,
													  true,
                                                      true, // CCW
                                                      revolution->isAVolume(),
                                                      _skeleton));

  GEOM_DISCRETIZER_UPDATE_CACHE(revolution);
  return true;
}


/* ----------------------------------------------------------------------- */

bool Discretizer::process( Swung * swung )
{
//#define TEST_CLOSURE
  GEOM_ASSERT(swung);

  GEOM_DISCRETIZER_CHECK_CACHE(swung);

  const ProfileInterpolationPtr& section= swung->getProfileInterpolation();
  GEOM_ASSERT(section);
  uint32_t sectionSize= section->getStride();
  uint32_t slices = swung->getSlices();

  Point3ArrayPtr pointList(new Point3Array(slices * sectionSize));
#ifdef TEST_CLOSURE
  Index3ArrayPtr indexList(new Index3Array((slices-1) * 2 * (sectionSize-1)));
#else
  Index3ArrayPtr indexList(new Index3Array( slices * 2 * (sectionSize-1)));
#endif

  const real_t angleMin= section->getUMin();
  const real_t angleMax= section->getUMax();
  uint32_t nbKnots= section->getKnotList()->getSize();
  const real_t range=
    ( nbKnots > 1 ) ? ( angleMax - angleMin ) : GEOM_TWO_PI;
#ifdef TEST_CLOSURE
  const real_t angleStep= range / (slices - 1);
#else
  const real_t angleStep= range / slices;
#endif

  uint32_t cur= 0;
  uint32_t next= sectionSize;

  real_t angle= angleMin;
  uint32_t pointsCount = 0;
  uint32_t facesCount = 0;
  uint32_t i= 0, j= 0;

  bool is2D= section->is2DInterpolMode();

  real_t cosa= 0., sina= 0., rad= 0.;
  Vector3 pt;
  Point2ArrayPtr crv2D;
  Point3ArrayPtr crv3D;

  for( i= 0; i < slices; i++ )
    {
    if(is2D)
      {
      crv2D= section->getSection2DAt(angle);
      cosa= cos(angle);
      sina= sin(angle);

      rad= crv2D->getAt(0).x();
      pt.x()= rad * cosa;
      pt.y()= rad * sina;
      pt.z()= crv2D->getAt(0).y();
      }
    else
      {
      crv3D= section->getSection3DAt(angle);
      pt= crv3D->getAt(0);
      }

    pointList->setAt(pointsCount++,pt);

    for( j= 1; j < sectionSize; j++ )
      {
      if( is2D )
        {
        rad= crv2D->getAt(j).x();
        pt.x()= rad * cosa;
        pt.y()= rad * sina;
        pt.z()= crv2D->getAt(j).y();
        }
      else
        pt= crv3D->getAt(j);

      pointList->setAt(pointsCount++,pt);

#ifdef TEST_CLOSURE
if( i != slices-1 )
  {
      indexList->setAt(facesCount++,
                       Index3(cur + j,cur + j - 1,next + j - 1));
      indexList->setAt(facesCount++,
                       Index3(cur + j,next + j - 1,next + j));
  }
#else
      indexList->setAt(facesCount++,
                       Index3(cur + j,cur + j - 1,next + j - 1));
      indexList->setAt(facesCount++,
                       Index3(cur + j,next + j - 1,next + j));
#endif
      }

    cur= next;
//    next= (next + sectionSize ) % (sectionSize * slices);
    next= (next + sectionSize ) % (sectionSize * slices);
    angle+= angleStep;

    }

  PolylinePtr skeleton(new Polyline(Vector3(0,0,0),
                                    Vector3(0,0,1)));

  // to do: gerer la fermeture (solid, volume)
  __discretization = ExplicitModelPtr(new TriangleSet(pointList,
                                                      indexList,
													  true,
                                                      swung->getCCW(), // CCW
                                                      false, // swung->isAVolume(),
                                                      skeleton));

  GEOM_DISCRETIZER_UPDATE_CACHE(swung);

#undef TEST_CLOSURE

  return true;
}

/* ----------------------------------------------------------------------- */


bool Discretizer::process( Scaled * scaled ) {
  GEOM_ASSERT(scaled);
  GEOM_DISCRETIZER_TRANSFORM(scaled );
  return true;
}


/* ----------------------------------------------------------------------- */


bool Discretizer::process( Sphere * sphere ) {
  GEOM_ASSERT(sphere);

  GEOM_DISCRETIZER_CHECK_CACHE_WITH_TEX(sphere);

  const real_t& _radius = sphere->getRadius();
  uchar_t _slices = sphere->getSlices();
  uchar_t _stacks = sphere->getStacks();

  uint32_t _ringCount = _stacks - 1;    // number of rings of points
  uint32_t _bot = _slices * _ringCount; // index of the lower point
  uint32_t _top = _bot + 1;             // index of the upper point

  Point3ArrayPtr _pointList = Point3ArrayPtr(new Point3Array(_top + 1));
  // Point3ArrayPtr _normalList = Point3ArrayPtr(new Point3Array(_top + 1));
  Index3ArrayPtr _indexList(new Index3Array(_slices * (2 * _ringCount)));

  real_t _azStep = GEOM_TWO_PI / _slices;
  real_t _elStep = GEOM_PI / _stacks;

  uint32_t _cur = 0;
  uint32_t _next = _ringCount;

  uint32_t _pointCount = 0;
  uint32_t _indexCount = 0;

  for (uint32_t _i = 0; _i < _slices; ++_i) {

    real_t _az = _i * _azStep;
    real_t _el = - GEOM_HALF_PI + _elStep;
    real_t _cosAz = cos(_az);
    real_t _sinAz = sin(_az);
    real_t _cosEl = cos(_el);
    real_t _x = _cosAz * _cosEl ;
    real_t _y = _sinAz * _cosEl ;
    real_t _z = sin(_el) ;

    _pointList->setAt(_pointCount++,Vector3(_x,_y,_z)* _radius);

    _indexList->setAt(_indexCount++,
                      Index3(_cur,_bot,_next));
    _indexList->setAt(_indexCount++,
                      Index3(_cur + _ringCount - 1,_next + _ringCount - 1,_top));

    for (uint32_t _j = 1; _j < _ringCount; ++_j) {
      _el += _elStep;
      _cosEl = cos(_el);
      _x = _cosAz * _cosEl ;
      _y = _sinAz * _cosEl ;
      _z = sin(_el) ;

      _pointList->setAt(_pointCount++,Vector3(_x,_y,_z)* _radius);

      _indexList->setAt(_indexCount++,
                        Index3(_cur + _j, _cur + _j - 1, _next + _j - 1));
      _indexList->setAt(_indexCount++,
                          Index3(_cur + _j, _next + _j - 1, _next + _j));
    };

    _cur = _next;
    _next = (_next + _ringCount ) % (_ringCount * _slices);
  };
  _pointList->setAt(_pointCount++,Vector3(0,0,-_radius));
  _pointList->setAt(_pointCount++,Vector3(0,0,_radius));

  GEOM_ASSERT(_pointCount == _pointList->getSize());
  GEOM_ASSERT(_indexCount == _indexList->getSize());

  PolylinePtr _skeleton(new Polyline(Vector3(_pointList->getAt(_bot)),
                                     Vector3(_pointList->getAt(_top))));

  TriangleSet * t = new TriangleSet(_pointList, _indexList, 0, 0 , 0, 0, /*_texList, _texIndexList,*/0,0, true, true,true,true,_skeleton);

  if(__computeTexCoord){
	  uchar_t _slices1 = _slices+1;
	  Point2ArrayPtr _texList(new Point2Array(_slices1 * (_stacks +1)));
	  _pointCount = 0;
	  for(uint32_t _i = 0; _i < _slices1 ; ++_i){
		  real_t _s = (real_t)_i/(real_t)_slices;
		  for (uint32_t _j = 1; _j < _stacks; ++_j) {
			  _texList->setAt(_pointCount++, Vector2(_s,(real_t)_j/(real_t)(_stacks+1)));
		  }
	  }
	  _bot = _pointCount;
	  for(uint32_t _i = 0; _i < _slices1 ; ++_i){
		  _texList->setAt(_pointCount++, Vector2((real_t)_i/(real_t)_slices,0));
	  }
	  _top = _pointCount;
	  for(uint32_t _i = 0; _i < _slices1 ; ++_i){
		  _texList->setAt(_pointCount++, Vector2((real_t)_i/(real_t)_slices,1));
	  }
	  t->getTexCoordList() = _texList;

      Index3ArrayPtr _texIndexList(new Index3Array(_slices * (2 * _ringCount)));
	  _indexCount = 0;
	  _cur = 0;
	  _next = _ringCount;
	  for(uint32_t _i = 0; _i < _slices ; ++_i){
		  _texIndexList->setAt(_indexCount++, Index3(_cur,_bot+_i,_next));
		  _texIndexList->setAt(_indexCount++,  Index3(_cur + _ringCount - 1,_next + _ringCount - 1,_top+_i));
		  for (uint32_t _j = 1; _j < _ringCount; ++_j) {
			  _texIndexList->setAt(_indexCount++,Index3(_cur + _j, _cur + _j - 1, _next + _j - 1));
			  _texIndexList->setAt(_indexCount++,Index3(_cur + _j, _next + _j - 1, _next + _j));
		  }
		  _cur = _next;
		  _next = _next + _ringCount ;
	  }
	  t->getTexCoordIndexList() = _texIndexList;
  }

  __discretization = ExplicitModelPtr(t);
  GEOM_DISCRETIZER_UPDATE_CACHE(sphere);
  return true;
}


/* ----------------------------------------------------------------------- */


bool Discretizer::process( Tapered * tapered ) {
  GEOM_ASSERT(tapered);
  GEOM_DISCRETIZER_CHECK_CACHE(tapered);
  if((tapered->getPrimitive())->apply(*this)){
      __discretization = __discretization->transform(tapered->getTransformation());
      GEOM_DISCRETIZER_UPDATE_CACHE(tapered);
  }
  else {
      __discretization = ExplicitModelPtr();
      return false;
  }
  return true;
}


/* ----------------------------------------------------------------------- */


bool Discretizer::process( Translated * translated ) {
  GEOM_ASSERT(translated);
  GEOM_DISCRETIZER_TRANSFORM(translated  );
  return true;
}



/* ----------------------------------------------------------------------- */


bool Discretizer::process( BezierCurve2D * bezierCurve ) {
  GEOM_ASSERT(bezierCurve);

  GEOM_DISCRETIZER_CHECK_CACHE(bezierCurve);

  real_t _start = 0;
  uint32_t _size = bezierCurve->getStride();
  real_t _step = 1.0 / (real_t)_size;
  Point3ArrayPtr _pointList(new Point3Array(_size + 1));

  for (uint32_t _i = 0; _i <= _size; _i++) {
    _pointList->setAt(_i,Vector3(bezierCurve->getPointAt(_start),0));
    _start += _step;
  };

  __discretization = ExplicitModelPtr(new Polyline(_pointList));

  GEOM_DISCRETIZER_UPDATE_CACHE(bezierCurve);
  return true;
}


/* ----------------------------------------------------------------------- */

bool Discretizer::process( Disc * disc ) {
  GEOM_ASSERT(disc);

  GEOM_DISCRETIZER_CHECK_CACHE_WITH_TEX(disc);

  real_t _radius = disc->getRadius();
  uint32_t _slices = disc->getSlices();

  Point3ArrayPtr _pointList(new Point3Array(_slices + 1));
  Point2ArrayPtr _texList(0);
  if(__computeTexCoord){
	  _texList = new Point2Array(_slices + 1);
  }
  Index3ArrayPtr _indexList(new Index3Array(_slices));

  uint32_t _cen = _slices;
  uint32_t _cur = 0;
  uint32_t _next = 1;

  uint32_t _pointsCount = 0;
  uint32_t _facesCount = 0;
  real_t _angleStep = GEOM_TWO_PI / _slices;

  for (uint32_t _i = 0; _i < _slices; _i++) {

    real_t _c = cos(_i * _angleStep);
	real_t _s = sin(_i * _angleStep);
    real_t _x = _c * _radius;
    real_t _y = _s * _radius;

	if(__computeTexCoord)
		_texList->setAt(_pointsCount,Vector2((_c/2) + 0.5,(_s/2)+0.5));
    _pointList->setAt(_pointsCount++,Vector3(_x,_y,0));
    _indexList->setAt(_facesCount++,Index3(_cur,_next,_cen));

    _cur = _next;
    _next = (_next + 1) % _slices;
  }
  if(__computeTexCoord)
	  _texList->setAt(_pointsCount,Vector2(0.5,0.5));

  PolylinePtr _skeleton(new Polyline(Vector3(0,0,0),
                                     Vector3(0,0,0)));

  TriangleSet * t = new TriangleSet(_pointList, _indexList, true, true,  false, _skeleton);
  if(__computeTexCoord)
		t->getTexCoordList() = _texList;

  __discretization = ExplicitModelPtr(t);

  GEOM_DISCRETIZER_UPDATE_CACHE(disc);
  return true;
}


/* ----------------------------------------------------------------------- */


bool Discretizer::process( NurbsCurve2D * nurbsCurve ) {
  GEOM_ASSERT(nurbsCurve);

  GEOM_DISCRETIZER_CHECK_CACHE(nurbsCurve);

  real_t _start = nurbsCurve->getFirstKnot();
  uint32_t _size = nurbsCurve->getStride();
  real_t _step =  (nurbsCurve->getLastKnot()-_start) / (real_t) _size;
  Point3ArrayPtr _pointList(new Point3Array(_size + 1));

  for (uint32_t _i = 0; _i < _size; _i++) {
    _pointList->setAt(_i,Vector3(nurbsCurve->getPointAt(_start),0));
    _start += _step;
  };

  _pointList->setAt(_size, Vector3(nurbsCurve->getPointAt(nurbsCurve->getLastKnot()),0));

  __discretization = ExplicitModelPtr(new Polyline(_pointList));

  GEOM_DISCRETIZER_UPDATE_CACHE(nurbsCurve);
  return true;
}

/* ----------------------------------------------------------------------- */

bool Discretizer::process( PointSet2D * pointSet ){
  GEOM_ASSERT(pointSet);

  GEOM_DISCRETIZER_CHECK_CACHE(pointSet);
  __discretization = ExplicitModelPtr(new PointSet(Point3ArrayPtr(new Point3Array(*(pointSet->getPointList()),0))));
  GEOM_DISCRETIZER_UPDATE_CACHE(pointSet);
  return true;
}


/* ----------------------------------------------------------------------- */

bool Discretizer::process( Polyline2D * polyline ){
  GEOM_ASSERT( polyline );


  GEOM_DISCRETIZER_CHECK_CACHE( polyline );
  Point3ArrayPtr a (new Point3Array(*(polyline->getPointList()),0));
  __discretization = ExplicitModelPtr(new Polyline(a));
  GEOM_DISCRETIZER_UPDATE_CACHE(polyline);
  return true;
}


/* ----------------------------------------------------------------------- */

Point2ArrayPtr 
Discretizer::gridTexCoord(Point3ArrayPtr pts, int gw, int gh) const {
  Point2ArrayPtr _texList = Point2ArrayPtr(new Point2Array(gw * gh));
  for ( int _u = 0 ; _u < gw ; _u ++){
	real_t length = 0;
	Vector3 p1 = pts->getAt(_u*gh);
	_texList->setAt(_u*gh,Vector2(0,0));
	{for (int _v = 1; _v < gh; _v ++) {
	  Vector3 p2 = pts->getAt(_u*gh+_v);
	  length += norm(p2-p1);
	  p1 = p2;
	}}
	real_t length2 = 0;
	p1 = pts->getAt(_u*gh);
	for (int _v = 1; _v < gh; _v ++) {
	  Vector3 p2 = pts->getAt(_u*gh+_v);
	  length2 += norm(p2-p1);
	  p1 = p2;
	  _texList->setAt(_u*gh+_v,Vector2(0,length2/length));
	}
  }
  for (int _v = 0; _v < gh; _v ++) {
	real_t length = 0;
	Vector3 p1 = pts->getAt(_v);
	{for ( int _u = 1 ; _u < gw ; _u ++){
	  Vector3 p2 = pts->getAt(_u*gh+_v);
	  length += norm(p2-p1);
	  p1 = p2;
	}}
	real_t length2 = 0;
	p1 = pts->getAt(_v);
	for ( int _u = 1 ; _u < gw ; _u ++){
	  Vector3 p2 = pts->getAt(_u*gh+_v);
	  length2 += norm(p2-p1);
	  p1 = p2;
	  _texList->getAt(_v+_u*gh).x() = length2/length;
	}
  }
  return _texList;
}

/* ----------------------------------------------------------------------- */

bool Discretizer::process( Text * text ) {
  GEOM_ASSERT(text);
  __discretization = ExplicitModelPtr();
  return false;
}

bool Discretizer::process( Font * font ) {
  GEOM_ASSERT(font);
  __discretization = ExplicitModelPtr();
  return false;
}

/* ----------------------------------------------------------------------- */