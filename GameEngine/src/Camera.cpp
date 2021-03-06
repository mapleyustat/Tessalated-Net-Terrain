#include "Camera.h"
#include <cmath>


UniformBufferObject<Camera::cameraMatrices>* Camera::cameraUBO = NULL;
Camera* Camera::current = NULL;

Camera::Camera(){
	if(cameraUBO == NULL){
		cameraUBO = new UniformBufferObject<Camera::cameraMatrices>("CamMats");
	}
	matrices.view = Matrix<4, 4>::Identity();
	matrices.proj = Matrix<4, 4>::Identity();

}

const Matrix<4, 4>& Camera::GetView(){
	return matrices.view;
}

const Matrix<4, 4>& Camera::GetProj(){
	return matrices.proj;
}
void Camera::LookAt(MVector<3> &position, MVector<3> &target, MVector<3> &Up){
	this->position = position;
	
	static MVector<4> lastColumn = {0, 0, 0, 1};

	//Used to ease inverting
	Matrix<4, 4> orientation;
	Matrix<4, 4> translation;

	//Get forward direction
	forward = position - target;
	forward.Normalize(); // In place normalization

	//Normalize up
	up = Up.Normalized(); // returns a normalized copy
	
	//Gram-Schmidt
	MVector<3> projection = forward * (up.Dot(forward));
	up = up - projection;
	up.Normalize();
	//Gram-Schmidt, considered doing it again to ensure precision
	//proj = forward * (up.Dot(forward));
	//up -= proj;

	//Get right vector
	right = up.Cross(forward * -1);
	
	//Set the orientation vector
	orientation.SetColumn(0, right.IncreasedDimensions<1>(0));	//IncreasedDimensions returns an MVector with a templated
																//Increase in dimensions and fills the new slots with the
																//Argument provided

																//SetColumn sets the values of the column to those of the MVector
	orientation.SetColumn(1, up.IncreasedDimensions<1>(0));
	orientation.SetColumn(2, (forward*-1).IncreasedDimensions<1>(0));
	orientation.SetColumn(3, lastColumn);	
	
	translation = Matrix<4, 4>::Identity();
	translation.SetColumn(3, position.IncreasedDimensions<1>(1)); // translation Matrix, simple enough
	
	world = translation.multiply(orientation); // This is the cameras object - world matrix

	translation.SetColumn(3, (position * -1).IncreasedDimensions<1>(1));  // Invert the translation matrix

	matrices.view = orientation.Transposed().multiply(translation); // Orientation matrix inverse == it's transpose.  Then reverse order
	//Matrix<4, 4> v = {1, 0, 0, 0, 0, 1, 0 ,0, 0,0, 1, 0, 0, 0, 0, 1}; 
	//matrices.view = v;
	//matrices.view.Transpose();
}
void Camera::SetProj(float nearClip, float farClip, float fieldOfViewY, MVector<2> &aspectRatio){
	zoomY = 1/tan(fieldOfViewY/2);
	aspect = aspectRatio;
	float zoomX = zoomY * aspectRatio.GetValue(1) /aspectRatio.GetValue(0);
	
	
	float projValues[16] = {	zoomX,	0,		0,		0,   
								0,		zoomY,	0,		0,	
								0,		0,		(farClip + nearClip)/( farClip- nearClip ), 1,
								0,		0,		(2 * nearClip * farClip) /( nearClip-farClip  ),	0}; 
	matrices.proj.SetValues(projValues);
	float invProjValues[16] = {
		1/zoomX, 0, 0, 0,
		0, 1/zoomY, 0, 0,
		0, 0, 0, (nearClip - farClip) / (2 * nearClip * farClip),
		0, 0, 1, (farClip + nearClip)/(2 * nearClip * farClip)};
	inverseProj.SetValues(invProjValues);
	MVector<4> yo = { 3, 2, 1, 1};
	yo = matrices.proj.multiply(inverseProj).multiply(yo);
	//Matrix<4, 4> v = {1, 0, 0, 0, 0, 1, 0 ,0, 0,0, 1, 0, 0, 0, 0, 1}; 
	//matrices.proj = v;
	//matrices.proj.Transpose();
	//proj.Transpose();

}
void Camera::SetCurrent(){
	Camera::cameraUBO->Update(&matrices);
	current = this;
}
MVector<3> Camera::Deproject(int x, int y)
{
	float normX = (2 * (float)x / aspect.GetValue(0)) -1;
	float normY = 1 - (2 * (float)y / aspect.GetValue(1));

	MVector<4> nearPlane = {normX, normY, 0, 1 };
	nearPlane = world.multiply(inverseProj).multiply(nearPlane);
	MVector<3> ret = { nearPlane.GetValue(0)/nearPlane.GetValue(3), nearPlane.GetValue(1)/nearPlane.GetValue(3), nearPlane.GetValue(2)/nearPlane.GetValue(3)  };
	ret = (ret - position).Normalized();
	return ret;
}