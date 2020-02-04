#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <mavros_msgs/CommandBool.h>
#include <mavros_msgs/SetMode.h>
#include <mavros_msgs/State.h>
#include<mavros_msgs/CommandTOL.h>
#include<nav_msgs/Odometry.h>
bool flag=false;
bool flag1=false;
float a,b,c,d,e,f;
void check(mavros_msgs::State msg){
     flag =msg.connected;
     flag1=msg.armed;
}
void status0(nav_msgs::Odometry msg1){
     a=msg1.pose.pose.position.x;
     b=msg1.pose.pose.position.y;
     c=msg1.pose.pose.position.z;
    ROS_INFO("%f %f %f ",a,b,c);
}
void status1(nav_msgs::Odometry msg1){
     d=msg1.pose.pose.position.x;
     e=msg1.pose.pose.position.y;
     f=msg1.pose.pose.position.z;
    ROS_INFO("%f %f %f ",d,e,f);
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "quad0");
    ros::NodeHandle nh;
   ros::Subscriber sub = nh.subscribe("uav0/mavros/state", 10, check);
   ros::Subscriber sub1 = nh.subscribe("uav0/mavros/local_position/odom", 10,status0);
   ros::Subscriber sub2 = nh.subscribe("uav1/mavros/local_position/odom", 10,status1);
   ros::Publisher local_pub = nh.advertise<geometry_msgs::PoseStamped> ("uav0/mavros/setpoint_position/local", 10);
    ros::ServiceClient setmode = nh.serviceClient<mavros_msgs::SetMode>("uav0/mavros/set_mode");
    ros::ServiceClient arming= nh.serviceClient<mavros_msgs::CommandBool>("uav0/mavros/cmd/arming");
    ros::ServiceClient takeoff= nh.serviceClient<mavros_msgs::CommandTOL>("uav0/mavros/cmd/takeoff");  
    ros::Rate rate(20);      
   while(ros::ok() &&!flag){
    ros::spinOnce();
    rate.sleep();
}
geometry_msgs::PoseStamped set;
    set.pose.position.x =0;
    set.pose.position.y = 0;
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
for(int i=0;ros::ok() && i < 10*20; ++i){
    local_pub.publish(set);
}
for(int i = 0; ros::ok() && i < 10*2000; ++i){
       if((fabs(a-d)>0.5)&&(fabs(b-e)>0.5)) local_pub.publish(set);
       else {
           set.pose.position.x =-(d-a);
    set.pose.position.y = e-b;
    set.pose.position.z = 2; 
           for (int i=0;i<100;i++){
    local_pub.publish(set);
    } 
       }
      ros::spinOnce();
      rate.sleep();
    }
  
    return 0;
}