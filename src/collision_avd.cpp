#include <time.h>
#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/TwistStamped.h>
#include <mavros_msgs/CommandBool.h>
#include <mavros_msgs/SetMode.h>
#include <mavros_msgs/State.h>
#include<mavros_msgs/CommandTOL.h>
#include<nav_msgs/Odometry.h>
/*bool flag=false;
bool flag1=false;
string mode_check;
void check(mavros_msgs::State msg){
     flag =msg.connected;
     flag1=msg.armed;
    mode_check=msg.mode;
}*/
mavros_msgs::State current_state;
float a,b,c,d,e,f,vel,vel_x,vel_y;
void check(const mavros_msgs::State::ConstPtr& msg){
    current_state = *msg;
}
void status0(nav_msgs::Odometry msg1){
     a=msg1.pose.pose.position.x;
     b=msg1.pose.pose.position.y;
     c=msg1.pose.pose.position.z;
    // ROS_INFO("%f %f",a,b);
}
void status1(nav_msgs::Odometry msg1){
     d=msg1.pose.pose.position.x;
     e=msg1.pose.pose.position.y;
     f=msg1.pose.pose.position.z;
     //ROS_INFO("%f %f ",d,e);
}
void status2(geometry_msgs::TwistStamped msg){
    vel_x=msg.twist.linear.x;
    vel_y=msg.twist.linear.y;
    vel=sqrt(vel_x*vel_x+vel_y*vel_y);
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "quad0");
    ros::NodeHandle nh;
    ros::Subscriber sub = nh.subscribe("uav0/mavros/state", 10, check);
    ros::Subscriber sub1 = nh.subscribe("uav0/mavros/local_position/odom", 10,status0);
    ros::Subscriber sub2 = nh.subscribe("uav1/mavros/local_position/odom", 10,status1);
    ros::Subscriber sub3 = nh.subscribe("uav1/mavros/local_position/velocity", 10,status2);
    ros::Publisher local_pub = nh.advertise<geometry_msgs::PoseStamped> ("uav0/mavros/setpoint_position/local", 10);
    ros::ServiceClient setmode = nh.serviceClient<mavros_msgs::SetMode>("uav0/mavros/set_mode");
    ros::ServiceClient arming= nh.serviceClient<mavros_msgs::CommandBool>("uav0/mavros/cmd/arming");  
    ros::Rate rate(30);      
    while(ros::ok() &&!current_state.connected){
        ros::spinOnce();
        rate.sleep();
    }
   
    mavros_msgs::CommandBool arm;
    arm.request.value = true;
    while(ros::ok()&&!current_state.armed){
        arming.call(arm);
        ros::spinOnce();
        rate.sleep();
    }
    geometry_msgs::PoseStamped set;
    set.pose.position.x=0;
    set.pose.position.y=0;
    set.pose.position.z=3;
    for(int i=0;ros::ok() && i < 100; ++i){
        local_pub.publish(set);
    }
    mavros_msgs::SetMode mode;
    mode.request.custom_mode="OFFBOARD";
    while(current_state.mode!="OFFBOARD"&&ros::ok()){
        setmode.call(mode);
        ros::spinOnce();
        rate.sleep();
    }
    while(ros::ok()){
            float rel_dist,flag;
           rel_dist=sqrt((a-d-10)*(a-d-10)+(b-e)*(b-e));
           flag=fabs(cos((vel_x*(a-d-10)+vel_y*(b-e))/vel*rel_dist));
           if((a-d-10)*(a-d-10)+(b-e)*(b-e)>(5+7*vel)||(flag<0.9)) {local_pub.publish(set);
           set.pose.position.x=0;
          set.pose.position.y=0;
          set.pose.position.z=3;}
          else{
              if(fabs(vel_y/vel_x)<fabs((b-e)/(a-d-10))&&fabs(vel_x*vel_y)>0.1*vel){
                if(vel_y*vel_x>0){
              set.pose.position.x=-(b-e);
              set.pose.position.y=(a-d-10);
              set.pose.position.z=3;}
              else if(vel_y*vel_x<0) {
              set.pose.position.x=(b-e);
              set.pose.position.y=-(a-d-10);
              set.pose.position.z=3;
              }
              }
              if(fabs(vel_y/vel_x)>=fabs((b-e)/(a-d-10))&&fabs(vel_x*vel_y)>0.1*vel){
                if(vel_y*vel_x<0){
              set.pose.position.x=-(b-e);
              set.pose.position.y=(a-d-10);
              set.pose.position.z=3;}
              else if(vel_y*vel_x>0) {
              set.pose.position.x=(b-e);
              set.pose.position.y=-(a-d-10);
              set.pose.position.z=3;
              }
              }
             else{
                  if(fabs(vel_x)<0.1*vel){
              set.pose.position.x=fabs(b-e)*(a-d-10)/fabs(a-d-10);
              set.pose.position.y=0;
              set.pose.position.z=3;
              }
               else if(fabs(vel_y)<0.1*vel){
              set.pose.position.x=0;
              set.pose.position.y=fabs(a-d-10)*(b-e)/fabs(b-e);
              set.pose.position.z=3;
              }
             }
           while((a-d-10)*(a-d-10)+(b-e)*(b-e)<(5+7*vel)&&(flag>0.9)){
               ROS_INFO("%f %f %f",flag,set.pose.position.x,set.pose.position.y);
               flag=fabs(cos((vel_x*(a-d-10)+vel_y*(b-e))/vel*rel_dist));
              local_pub.publish(set);
              ros::spinOnce();
          rate.sleep();
           }
          }
          ros::spinOnce();
          rate.sleep();
          
        }
        return 0;
}