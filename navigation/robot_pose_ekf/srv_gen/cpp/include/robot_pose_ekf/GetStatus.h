/* Auto-generated by genmsg_cpp for file /home/harlie/code/dev_stacks/navigation/robot_pose_ekf/srv/GetStatus.srv */
#ifndef ROBOT_POSE_EKF_SERVICE_GETSTATUS_H
#define ROBOT_POSE_EKF_SERVICE_GETSTATUS_H
#include <string>
#include <vector>
#include <ostream>
#include "ros/serialization.h"
#include "ros/builtin_message_traits.h"
#include "ros/message_operations.h"
#include "ros/message.h"
#include "ros/time.h"

#include "ros/service_traits.h"




namespace robot_pose_ekf
{
template <class ContainerAllocator>
struct GetStatusRequest_ : public ros::Message
{
  typedef GetStatusRequest_<ContainerAllocator> Type;

  GetStatusRequest_()
  {
  }

  GetStatusRequest_(const ContainerAllocator& _alloc)
  {
  }


private:
  static const char* __s_getDataType_() { return "robot_pose_ekf/GetStatusRequest"; }
public:
  ROSCPP_DEPRECATED static const std::string __s_getDataType() { return __s_getDataType_(); }

  ROSCPP_DEPRECATED const std::string __getDataType() const { return __s_getDataType_(); }

private:
  static const char* __s_getMD5Sum_() { return "d41d8cd98f00b204e9800998ecf8427e"; }
public:
  ROSCPP_DEPRECATED static const std::string __s_getMD5Sum() { return __s_getMD5Sum_(); }

  ROSCPP_DEPRECATED const std::string __getMD5Sum() const { return __s_getMD5Sum_(); }

private:
  static const char* __s_getServerMD5Sum_() { return "4fe5af303955c287688e7347e9b00278"; }
public:
  ROSCPP_DEPRECATED static const std::string __s_getServerMD5Sum() { return __s_getServerMD5Sum_(); }

  ROSCPP_DEPRECATED const std::string __getServerMD5Sum() const { return __s_getServerMD5Sum_(); }

private:
  static const char* __s_getMessageDefinition_() { return "\n\
\n\
"; }
public:
  ROSCPP_DEPRECATED static const std::string __s_getMessageDefinition() { return __s_getMessageDefinition_(); }

  ROSCPP_DEPRECATED const std::string __getMessageDefinition() const { return __s_getMessageDefinition_(); }

  ROSCPP_DEPRECATED virtual uint8_t *serialize(uint8_t *write_ptr, uint32_t seq) const
  {
    ros::serialization::OStream stream(write_ptr, 1000000000);
    return stream.getData();
  }

  ROSCPP_DEPRECATED virtual uint8_t *deserialize(uint8_t *read_ptr)
  {
    ros::serialization::IStream stream(read_ptr, 1000000000);
    return stream.getData();
  }

  ROSCPP_DEPRECATED virtual uint32_t serializationLength() const
  {
    uint32_t size = 0;
    return size;
  }

  typedef boost::shared_ptr< ::robot_pose_ekf::GetStatusRequest_<ContainerAllocator> > Ptr;
  typedef boost::shared_ptr< ::robot_pose_ekf::GetStatusRequest_<ContainerAllocator>  const> ConstPtr;
}; // struct GetStatusRequest
typedef  ::robot_pose_ekf::GetStatusRequest_<std::allocator<void> > GetStatusRequest;

typedef boost::shared_ptr< ::robot_pose_ekf::GetStatusRequest> GetStatusRequestPtr;
typedef boost::shared_ptr< ::robot_pose_ekf::GetStatusRequest const> GetStatusRequestConstPtr;


template <class ContainerAllocator>
struct GetStatusResponse_ : public ros::Message
{
  typedef GetStatusResponse_<ContainerAllocator> Type;

  GetStatusResponse_()
  : status()
  {
  }

  GetStatusResponse_(const ContainerAllocator& _alloc)
  : status(_alloc)
  {
  }

  typedef std::basic_string<char, std::char_traits<char>, typename ContainerAllocator::template rebind<char>::other >  _status_type;
  std::basic_string<char, std::char_traits<char>, typename ContainerAllocator::template rebind<char>::other >  status;


private:
  static const char* __s_getDataType_() { return "robot_pose_ekf/GetStatusResponse"; }
public:
  ROSCPP_DEPRECATED static const std::string __s_getDataType() { return __s_getDataType_(); }

  ROSCPP_DEPRECATED const std::string __getDataType() const { return __s_getDataType_(); }

private:
  static const char* __s_getMD5Sum_() { return "4fe5af303955c287688e7347e9b00278"; }
public:
  ROSCPP_DEPRECATED static const std::string __s_getMD5Sum() { return __s_getMD5Sum_(); }

  ROSCPP_DEPRECATED const std::string __getMD5Sum() const { return __s_getMD5Sum_(); }

private:
  static const char* __s_getServerMD5Sum_() { return "4fe5af303955c287688e7347e9b00278"; }
public:
  ROSCPP_DEPRECATED static const std::string __s_getServerMD5Sum() { return __s_getServerMD5Sum_(); }

  ROSCPP_DEPRECATED const std::string __getServerMD5Sum() const { return __s_getServerMD5Sum_(); }

private:
  static const char* __s_getMessageDefinition_() { return "string status\n\
\n\
"; }
public:
  ROSCPP_DEPRECATED static const std::string __s_getMessageDefinition() { return __s_getMessageDefinition_(); }

  ROSCPP_DEPRECATED const std::string __getMessageDefinition() const { return __s_getMessageDefinition_(); }

  ROSCPP_DEPRECATED virtual uint8_t *serialize(uint8_t *write_ptr, uint32_t seq) const
  {
    ros::serialization::OStream stream(write_ptr, 1000000000);
    ros::serialization::serialize(stream, status);
    return stream.getData();
  }

  ROSCPP_DEPRECATED virtual uint8_t *deserialize(uint8_t *read_ptr)
  {
    ros::serialization::IStream stream(read_ptr, 1000000000);
    ros::serialization::deserialize(stream, status);
    return stream.getData();
  }

  ROSCPP_DEPRECATED virtual uint32_t serializationLength() const
  {
    uint32_t size = 0;
    size += ros::serialization::serializationLength(status);
    return size;
  }

  typedef boost::shared_ptr< ::robot_pose_ekf::GetStatusResponse_<ContainerAllocator> > Ptr;
  typedef boost::shared_ptr< ::robot_pose_ekf::GetStatusResponse_<ContainerAllocator>  const> ConstPtr;
}; // struct GetStatusResponse
typedef  ::robot_pose_ekf::GetStatusResponse_<std::allocator<void> > GetStatusResponse;

typedef boost::shared_ptr< ::robot_pose_ekf::GetStatusResponse> GetStatusResponsePtr;
typedef boost::shared_ptr< ::robot_pose_ekf::GetStatusResponse const> GetStatusResponseConstPtr;

struct GetStatus
{

typedef GetStatusRequest Request;
typedef GetStatusResponse Response;
Request request;
Response response;

typedef Request RequestType;
typedef Response ResponseType;
}; // struct GetStatus
} // namespace robot_pose_ekf

namespace ros
{
namespace message_traits
{
template<class ContainerAllocator>
struct MD5Sum< ::robot_pose_ekf::GetStatusRequest_<ContainerAllocator> > {
  static const char* value() 
  {
    return "d41d8cd98f00b204e9800998ecf8427e";
  }

  static const char* value(const  ::robot_pose_ekf::GetStatusRequest_<ContainerAllocator> &) { return value(); } 
  static const uint64_t static_value1 = 0xd41d8cd98f00b204ULL;
  static const uint64_t static_value2 = 0xe9800998ecf8427eULL;
};

template<class ContainerAllocator>
struct DataType< ::robot_pose_ekf::GetStatusRequest_<ContainerAllocator> > {
  static const char* value() 
  {
    return "robot_pose_ekf/GetStatusRequest";
  }

  static const char* value(const  ::robot_pose_ekf::GetStatusRequest_<ContainerAllocator> &) { return value(); } 
};

template<class ContainerAllocator>
struct Definition< ::robot_pose_ekf::GetStatusRequest_<ContainerAllocator> > {
  static const char* value() 
  {
    return "\n\
\n\
";
  }

  static const char* value(const  ::robot_pose_ekf::GetStatusRequest_<ContainerAllocator> &) { return value(); } 
};

template<class ContainerAllocator> struct IsFixedSize< ::robot_pose_ekf::GetStatusRequest_<ContainerAllocator> > : public TrueType {};
} // namespace message_traits
} // namespace ros


namespace ros
{
namespace message_traits
{
template<class ContainerAllocator>
struct MD5Sum< ::robot_pose_ekf::GetStatusResponse_<ContainerAllocator> > {
  static const char* value() 
  {
    return "4fe5af303955c287688e7347e9b00278";
  }

  static const char* value(const  ::robot_pose_ekf::GetStatusResponse_<ContainerAllocator> &) { return value(); } 
  static const uint64_t static_value1 = 0x4fe5af303955c287ULL;
  static const uint64_t static_value2 = 0x688e7347e9b00278ULL;
};

template<class ContainerAllocator>
struct DataType< ::robot_pose_ekf::GetStatusResponse_<ContainerAllocator> > {
  static const char* value() 
  {
    return "robot_pose_ekf/GetStatusResponse";
  }

  static const char* value(const  ::robot_pose_ekf::GetStatusResponse_<ContainerAllocator> &) { return value(); } 
};

template<class ContainerAllocator>
struct Definition< ::robot_pose_ekf::GetStatusResponse_<ContainerAllocator> > {
  static const char* value() 
  {
    return "string status\n\
\n\
";
  }

  static const char* value(const  ::robot_pose_ekf::GetStatusResponse_<ContainerAllocator> &) { return value(); } 
};

} // namespace message_traits
} // namespace ros

namespace ros
{
namespace serialization
{

template<class ContainerAllocator> struct Serializer< ::robot_pose_ekf::GetStatusRequest_<ContainerAllocator> >
{
  template<typename Stream, typename T> inline static void allInOne(Stream& stream, T m)
  {
  }

  ROS_DECLARE_ALLINONE_SERIALIZER;
}; // struct GetStatusRequest_
} // namespace serialization
} // namespace ros


namespace ros
{
namespace serialization
{

template<class ContainerAllocator> struct Serializer< ::robot_pose_ekf::GetStatusResponse_<ContainerAllocator> >
{
  template<typename Stream, typename T> inline static void allInOne(Stream& stream, T m)
  {
    stream.next(m.status);
  }

  ROS_DECLARE_ALLINONE_SERIALIZER;
}; // struct GetStatusResponse_
} // namespace serialization
} // namespace ros

namespace ros
{
namespace service_traits
{
template<>
struct MD5Sum<robot_pose_ekf::GetStatus> {
  static const char* value() 
  {
    return "4fe5af303955c287688e7347e9b00278";
  }

  static const char* value(const robot_pose_ekf::GetStatus&) { return value(); } 
};

template<>
struct DataType<robot_pose_ekf::GetStatus> {
  static const char* value() 
  {
    return "robot_pose_ekf/GetStatus";
  }

  static const char* value(const robot_pose_ekf::GetStatus&) { return value(); } 
};

template<class ContainerAllocator>
struct MD5Sum<robot_pose_ekf::GetStatusRequest_<ContainerAllocator> > {
  static const char* value() 
  {
    return "4fe5af303955c287688e7347e9b00278";
  }

  static const char* value(const robot_pose_ekf::GetStatusRequest_<ContainerAllocator> &) { return value(); } 
};

template<class ContainerAllocator>
struct DataType<robot_pose_ekf::GetStatusRequest_<ContainerAllocator> > {
  static const char* value() 
  {
    return "robot_pose_ekf/GetStatus";
  }

  static const char* value(const robot_pose_ekf::GetStatusRequest_<ContainerAllocator> &) { return value(); } 
};

template<class ContainerAllocator>
struct MD5Sum<robot_pose_ekf::GetStatusResponse_<ContainerAllocator> > {
  static const char* value() 
  {
    return "4fe5af303955c287688e7347e9b00278";
  }

  static const char* value(const robot_pose_ekf::GetStatusResponse_<ContainerAllocator> &) { return value(); } 
};

template<class ContainerAllocator>
struct DataType<robot_pose_ekf::GetStatusResponse_<ContainerAllocator> > {
  static const char* value() 
  {
    return "robot_pose_ekf/GetStatus";
  }

  static const char* value(const robot_pose_ekf::GetStatusResponse_<ContainerAllocator> &) { return value(); } 
};

} // namespace service_traits
} // namespace ros

#endif // ROBOT_POSE_EKF_SERVICE_GETSTATUS_H

