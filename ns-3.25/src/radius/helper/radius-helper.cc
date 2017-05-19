/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "radius-helper.h"
#include "ns3/radius-module.h"

namespace ns3
{

RadiusHelper::RadiusHelper(void)
{
  m_srv_factory.SetTypeId ("ns3::RadiusServer");
  m_client_factory.SetTypeId ("ns3::RadiusClient");
}

void RadiusHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  bool set_server = true;
  bool set_client = true;

  /* filter exclusive parameters */
  if (name == "ServerDelay")
    set_client = false;
  else if (name == "ServerAddress")
    set_server = false;

  if (set_server)
    m_srv_factory.Set (name, value);
  if (set_client)
    m_client_factory.Set (name, value);
}

void RadiusHelper::SetDatabase(Ptr<RadiusDB> database)
{
  m_rad_db = database;
}

ApplicationContainer RadiusHelper::InstallClient (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (AttachClient (*i));
    }

  return apps;
}

ApplicationContainer RadiusHelper::InstallClient (Ptr<Node> node) const
{
  return ApplicationContainer (AttachClient (node));
}

ApplicationContainer RadiusHelper::InstallServer (Ptr<Node> node) const
{
  return ApplicationContainer (AttachServer (node));
}

Ptr<Application> RadiusHelper::AttachClient (Ptr<Node> node) const
{
  Ptr<Application> app = m_client_factory.Create<Application> ();
  node->AddApplication (app);

  return app;
}

Ptr<Application> RadiusHelper::AttachServer (Ptr<Node> node) const
{
  Ptr<Application> app = m_srv_factory.Create<Application> ();
  Ptr<RadiusServer> srv = DynamicCast<RadiusServer>(app);
  srv->SetDatabase(m_rad_db);
  node->AddApplication (app);

  return app;
}

}

