/**
 * @file offb_node.cpp
 * @brief Offboard control example node, written with MAVROS version 0.19.x, PX4 Pro Flight
 * Stack and tested in Gazebo SITL
 */

#include <ros/ros.h>
//#include <geometry_msgs/PoseWithCovarianceStamped.h>
//#include <mavros_msgs/State.h>
//#include <sensor_msgs/Imu.h>
#include <geometry_msgs/PointStamped.h>
#include <geometry_msgs/PoseStamped.h>

using namespace std;

//mavros_msgs::State current_state;
//sensor_msgs::Imu current_imu;
geometry_msgs::PointStamped current_targetposMsg;
//geometry_msgs::PoseWithCovarianceStamped current_pose;
geometry_msgs::PoseStamped current_droneposMsg;

//void state_cb(const mavros_msgs::State::ConstPtr& msg){
//    current_state = *msg;
//}


double toEulerAngle(double q_w, double q_x, double q_y, double q_z)
{
  // roll (x-axis rotation)
  double sinr_cosp = +2.0 * (q_w * q_x + q_y * q_z);
  double cosr_cosp = +1.0 - 2.0 * (q_x * q_x + q_y * q_y);
  double roll = atan2(sinr_cosp, cosr_cosp);

  // pitch (y-axis rotation)
  double sinp = +2.0 * (q_w * q_y - q_z * q_x);
  if (fabs(sinp) >= 1)
  double   pitch = copysign(M_PI / 2, sinp); // use 90 degrees if out of range
  else
  double   pitch = asin(sinp);

  // yaw (z-axis rotation)
  double siny_cosp = +2.0 * (q_w * q_z + q_x * q_y);
  double cosy_cosp = +1.0 - 2.0 * (q_y * q_y + q_z * q_z);  
  double yaw = atan2(siny_cosp, cosy_cosp);
  return yaw;
}

double frame_conversion_yaw_x(double input_yaw,double input_x, double input_y)
{
  //convert body frame to local frame
  return input_x * cos(input_yaw) - input_y * sin(input_yaw);
}

double frame_conversion_yaw_y(double input_yaw,double input_x, double input_y)
{
  //convert body frame to local frame
  return input_x * sin(input_yaw) + input_y * cos(input_yaw);
}

//void posewithcovariancestamped_cb(const geometry_msgs::PoseWithCovarianceStamped::ConstPtr& msg){
//    current_pose = *msg;
//}

void posestamped_cb(const geometry_msgs::PoseStamped::ConstPtr& msg){
    current_droneposMsg = *msg;
}

//void imu_cb(const sensor_msgs::Imu::ConstPtr& msg){
//    current_imu = *msg;
//}

void pointstamped_cb(const geometry_msgs::PointStamped::ConstPtr& msg){
    current_targetposMsg = *msg;
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "offb_node");
    ros::NodeHandle nh;

//    ros::Subscriber state_sub = nh.subscribe<mavros_msgs::State>
//            ("mavros/state", 10, state_cb);

//    ros::Subscriber imu_sub = nh.subscribe<sensor_msgs::Imu>
//            ("mavros/imu/data", 10, imu_cb);

    ros::Subscriber pointstamped_sub = nh.subscribe<geometry_msgs::PointStamped>
            ("serial_ti_radar_read/radar_info0", 10, pointstamped_cb);

//    ros::Subscriber posewithcovariancestamped_sub = nh.subscribe<geometry_msgs::PoseWithCovarianceStamped>
//            ("mavros/global_position/local", 10, posewithcovariancestamped_cb);

    ros::Subscriber posestamped_sub = nh.subscribe<geometry_msgs::PoseStamped>
            ("mavros/local_position/pose", 10, posestamped_cb);

    //the setpoint publishing rate MUST be faster than 2Hz
    ros::Rate rate(20.0);

    ros::Time last_request = ros::Time::now();

    double yaw_angle;
    double pos_x, pos_y;
    double posX, posY;

    while(ros::ok()){

        yaw_angle = toEulerAngle(current_droneposMsg.pose.orientation.w, current_droneposMsg.pose.orientation.x, current_droneposMsg.pose.orientation.y, current_droneposMsg.pose.orientation.z);//(current_imu.orientation.w, current_imu.orientation.x,current_imu.orientation.y,current_imu.orientation.z);
        printf("yaw:   %lf       ", yaw_angle * 180/ 3.1415926);
        
        posX = current_targetposMsg.point.y;
        posY = current_targetposMsg.point.x;
        pos_x = frame_conversion_yaw_x(yaw_angle, posX, posY);
        pos_y = frame_conversion_yaw_y(yaw_angle, posX, posY);

//        printf("x = %lf    y = %lf    \n", pos_x, pos_y);

//        printf("\n\nx = %lf    y = %lf    \n\n", current_targetposMsg.point.x, current_targetposMsg.point.y);

        
        printf("\n\nx = %lf    y = %lf    ", current_droneposMsg.pose.position.x, current_droneposMsg.pose.position.y);
        printf("x = %lf    y = %lf    ", current_droneposMsg.pose.position.x + pos_x, current_droneposMsg.pose.position.y + pos_y);

        ros::spinOnce();
        rate.sleep();
    }
    return 0;
}

