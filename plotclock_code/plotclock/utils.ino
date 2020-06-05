// get 3rd triangle side by 2 sides and angle between them
// note: doesTriangleAngleExist should be called first
// if it's unknown whether dAngle exists in triangle
// (if it doesn't exist result will be incorrect)
float getSideBy2SidesAndAngle(const float a, const float b, const float dAngle) {
  const float rAngle = radians(dAngle);
  return sqrt(sq(a) + sq(b) - 2 * a * b * cos(rAngle));
}

// get triangle angle between a and b sides
// note: doesTriangleExist should be called first
// if it's unknown whether such triangle exists
// (if it doesn't exist result will be incorrect)
float getAngleBy3Sides(const float a, const float b, const float c) {
  const float rAngle = acos((sq(a) + sq(b) - sq(c)) / (2 * a * b));
  return degrees(rAngle);
}

bool doesTriangleAngleExist(const float dAngle) {
  return !(dAngle < 0 || dAngle > 180);
}

bool doesTriangleExist(const float a, const float b, const float c) {
  return !(
    (a + b) < c
    || (a + c) < b
    || (b + c) < a
  );
}

// get line point y value.
// (x1,y1) - line start point
// (x2,y2) - line end point
float getLineY(const float x, const float x1, const float y1, const float x2, const float y2) {
  return (x - x1) * (y2 - y1) / (x2 - x1) + y1;
}

// get absolute y value of half-ellipse.
// a - semi-major axis value
// b - semi-minor axis value
// note: ellipse center is (0,0) point of coordinate plane
float getHalfEllipseAbsY(const float x, const float a, const float b) {
  return b * sqrt(1 - (sq(x) / sq(a)));
}

// get absolute y value of half-circle.
// r - radius
// note: circle center is (0,0) point of coordinate plane
float getHalfCircleAbsY(const float x, const float r) {
  return sqrt(sq(r) - sq(x));
}
