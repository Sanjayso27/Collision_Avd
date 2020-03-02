#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <mavros_msgs/CommandBool.h>
#include <mavros_msgs/SetMode.h>
#include <mavros_msgs/State.h>
#include<mavros_msgs/CommandTOL.h>
#include<nav_msgs/Odometry.h>
bool flag=false;
bool flag1=false;
void check(mavros_msgs::State msg){
     flag =msg.connected;
     flag1=msg.armed;
}
int main(int argc, char **argv)
{
    ros::init(argc, argv, "quad1");
    ros::NodeHandle nh;
   ros::Subscriber sub = nh.subscribe("uav1/mavros/state", 10, check);
   ros::Publisher local_pub = nh.advertise<geometry_msgs::PoseStamped> ("uav1/mavros/setpoint_position/local", 10);
    ros::ServiceClient setmode = nh.serviceClient<mavros_msgs::SetMode>("uav1/mavros/set_mode");
    ros::ServiceClient arming= nh.serviceClient<mavros_msgs::CommandBool>("uav1/mavros/cmd/arming");
    ros::ServiceClient takeoff= nh.serviceClient<mavros_msgs::CommandTOL>("uav1/mavros/cmd/takeoff");  
    ros::Rate rate(20);      
   while(ros::ok() &&!flag){
    ros::spinOnce();
    rate.sleep();
}
geometry_msgs::PoseStamped set;
    set.pose.position.x =1;
    set.pose.position.y = 1;
    set.pose.position.z = 2;
    for(int i = 100; ros::ok() && i > 0; --i){
        local_pub.publish(set);
        ros::spinOnce();
        rate.sleep();
    }
mavros_msgs::SetMode mode;
mode.request.custom_mode="OFFBOARD";
mavros_msgs::CommandBool arm;
arm.request.value = true;
mavros_msgs::CommandTOL take;
while(ros::ok()&&!flag1){
    setmode.call(mode);
    arming.call(arm);
    local_pub.publish(set);
    ros::spinOnce();
    rate.sleep();
}
for(int i = 0; ros::ok() && i < 10*20; ++i){
      local_pub.publish(set);
      ros::spinOnce();
      rate.sleep();
    }
  set.pose.position.x =-1;
    set.pose.position.y = -1;
    set.pose.position.z = 2;
for(int i = 0; ros::ok() && i < 10*20; ++i){
      local_pub.publish(set);
      ros::spinOnce();
      rate.sleep();
    }
    return 0;
}