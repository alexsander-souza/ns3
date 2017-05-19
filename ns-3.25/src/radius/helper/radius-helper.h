/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef RADIUS_HELPER_H
#define RADIUS_HELPER_H

#include <string>
#include "ns3/object-factory.h"
#include "ns3/attribute.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include <ns3/ptr.h>
#include <ns3/radius-db.h>

namespace ns3
{

/**
 * \ingroup Radius
 * \brief A helper to make it easier to instantiate a ns3::RadiusClient or
 * ns3::RadiusServer on a set of nodes.
 */
class RadiusHelper
{
public:
  /**
   * Create a RADIUS protocol helper
   */
  RadiusHelper(void);

  /**
   * Helper function used to set the underlying application attributes.
   *
   * \param name the name of the application attribute to set
   * \param value the value of the application attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
   * Helper function used to link to the database
   *
   * \param database the database pointer
   */
  void SetDatabase(Ptr<RadiusDB> database);

  /**
   * Install an ns3::RadiusClient on each node of the input container
   * configured with all the attributes set with SetAttribute.
   *
   * \param c NodeContainer of the set of nodes on which an RadiusClient
   * will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer InstallClient (NodeContainer c) const;

  /**
   * Install an ns3::RadiusClient on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an RadiusClient will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer InstallClient (Ptr<Node> node) const;

  /**
   * Install an ns3::RadiusServer on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an RadiusServer will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer InstallServer (Ptr<Node> node) const;

private:
  Ptr<Application> AttachClient (Ptr<Node> node) const;
  Ptr<Application> AttachServer (Ptr<Node> node) const;

  ObjectFactory  m_srv_factory; //!< Object factory.
  ObjectFactory  m_client_factory; //!< Object factory.
  Ptr<RadiusDB>  m_rad_db; //!< Database pointer
};

}

#endif /* RADIUS_HELPER_H */

