#include <TFmini.h>
// #include <geometry_msgs/PoseStamped.h>
#include <nav_msgs/Odometry.h>

//odom_cb
nav_msgs::Odometry px4_odom;
void px4_odom_cb(const nav_msgs::Odometry& msg)
{
  px4_odom = msg;
}


int main(int argc, char **argv)
{
  ros::init(argc, argv, "tfmini_ros_node");
  ros::NodeHandle nh;
  std::string id = "TFmini";
  std::string portName;
  int baud_rate;
  benewake::TFmini *tfmini_obj;

  //New
  ros::Subscriber px4_odom_sub = nh.subscribe("pilot/local_position/odom", 100, px4_odom_cb);
  ros::Publisher tfmini_odom_pub = nh.advertise<nav_msgs::Odometry>("tfmini_odom",10);

  nh.param("serial_port", portName, std::string("/dev/ttyUSB0"));
  nh.param("baud_rate", baud_rate, 115200);

  tfmini_obj = new benewake::TFmini(portName, baud_rate);
  ros::Publisher pub_range = nh.advertise<sensor_msgs::Range>(id, 1000, true);
  sensor_msgs::Range TFmini_range;
  TFmini_range.radiation_type = sensor_msgs::Range::INFRARED;
  TFmini_range.field_of_view = 0.04;
  TFmini_range.min_range = 0.3;
  TFmini_range.max_range = 12;
  TFmini_range.header.frame_id = id;
  float dist = 0;
  ROS_INFO_STREAM("Start processing ...");

  while(ros::master::check() && ros::ok())
  {
    ros::spinOnce();
    dist = tfmini_obj->getDist();
    if(dist > 0 && dist < TFmini_range.max_range)
    {
      TFmini_range.range = dist;
      TFmini_range.header.stamp = ros::Time::now();
      pub_range.publish(TFmini_range);

      //New
      nav_msgs::Odometry odom_;
      odom_.pose = px4_odom.pose;
      odom_.twist = px4_odom.twist;
      // odom_.covariance = px4_odom.covariance;
      odom_.header.frame_id = px4_odom.header.frame_id;
      odom_.child_frame_id = px4_odom.child_frame_id;
      odom_.header.stamp = ros::Time::now();
      odom_.pose.pose.position.z = TFmini_range.range;
      tfmini_odom_pub.publish(odom_);
    }
    else if(dist == -1.0)
    {
      ROS_ERROR_STREAM("Failed to read data. TFmini ros node stopped!");
      break;
    }
    else if(dist == 0.0)
    {
      ROS_ERROR_STREAM("Data validation error!");
    }

    
  }

  tfmini_obj->closePort();
}
