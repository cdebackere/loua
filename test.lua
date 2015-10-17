-- initialize the server
ua = require("open62541")
server = ua.Server(16663)

-- entity objecttype
local requested_nodeid = ua.nodeids.null
local parent_nodeid = ua.nodeids.baseobjecttype
local reference_nodeid = ua.nodeids.hassubtype
local browsename = ua.types.QualifiedName(1, "Entity")
local attr = ua.types.ObjectTypeAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Entity")
local entity_id = server:addObjectTypeNode(requested_nodeid, parent_nodeid,
                                           reference_nodeid, browsename, attr)

-- interface objecttype
local requested_nodeid = ua.nodeids.null
local parent_nodeid = ua.nodeids.baseobjecttype
local reference_nodeid = ua.nodeids.hassubtype
local browsename = ua.types.QualifiedName(1, "Interface")
local attr = ua.types.ObjectTypeAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Interface")
local interface_id = server:addObjectTypeNode(requested_nodeid, parent_nodeid,
                                              reference_nodeid, browsename, attr)

-- connectedwith referencetype
local requested_nodeid = ua.nodeids.null
local parent_nodeid = ua.nodeids.nonhierarchicalreferences
local reference_nodeid = ua.nodeids.hassubtype
local browsename = ua.types.QualifiedName(1, "ConnectedWith")
local attr = ua.types.ReferenceTypeAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "ConnectedWith")
attr.symmetric = true
connectedwith_id = server:addReferenceTypeNode(requested_nodeid, parent_nodeid,
                                               reference_nodeid, browsename, attr)

-- administrationshell objecttype
local requested_nodeid = ua.nodeids.null
local parent_nodeid = ua.nodeids.baseobjecttype
local reference_nodeid = ua.nodeids.hassubtype
local browsename = ua.types.QualifiedName(1, "AdministrationShell")
local attr = ua.types.ObjectTypeAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "AdministrationShell")
local as_id = server:addObjectTypeNode(requested_nodeid, parent_nodeid,
                                           reference_nodeid, browsename, attr)

-- administrates referencetype
local requested_nodeid = ua.nodeids.null
local parent_nodeid = ua.nodeids.nonhierarchicalreferences
local reference_nodeid = ua.nodeids.hassubtype
local browsename = ua.types.QualifiedName(1, "Administrates")
local attr = ua.types.ReferenceTypeAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Administrates")
attr.inversename = ua.types.LocalizedText("en_US", "AdministeredBy")
local administrates_id = server:addReferenceTypeNode(requested_nodeid, parent_nodeid,
                                                     reference_nodeid, browsename, attr)

-- usb interface
local requested_nodeid = ua.nodeids.null
local parent_nodeid = interface_id
local reference_nodeid = ua.nodeids.hassubtype
local browsename = ua.types.QualifiedName(1, "USBInterface")
local attr = ua.types.ObjectTypeAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "USBInterface")
local usb_id = server:addObjectTypeNode(requested_nodeid, parent_nodeid,
                                              reference_nodeid, browsename, attr)

local requested_nodeid = ua.nodeids.null
local parent_nodeid = usb_id
local reference_nodeid = ua.nodeids.hasproperty
local browsename = ua.types.QualifiedName(1, "USBVersion")
local typeidentifier = ua.nodeids.null
local attr = ua.types.VariableAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "USBVersion")
id = server:addVariableNode(requested_nodeid, parent_nodeid, reference_nodeid,
                            browsename, typeidentifier, attr)

local requested_nodeid = ua.nodeids.null
local parent_nodeid = usb_id
local reference_nodeid = ua.nodeids.hasproperty
local browsename = ua.types.QualifiedName(1, "USBConnectorType")
local typeidentifier = ua.nodeids.null
local attr = ua.types.VariableAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "USBConnectorType")
id = server:addVariableNode(requested_nodeid, parent_nodeid, reference_nodeid,
                            browsename, typeidentifier, attr)

-- entities
local requested_nodeid = ua.nodeids.null
local parent_nodeid = ua.nodeids.objects
local reference_nodeid = ua.nodeids.organizes
local browsename = ua.types.QualifiedName(1, "Entities")
local typeidentifier = ua.nodeids.foldertype
local attr = ua.types.ObjectAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Entities")
entities_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                 reference_nodeid, browsename, typeidentifier, attr)

-- office
local requested_nodeid = ua.nodeids.null
local parent_nodeid = entities_id
local reference_nodeid = ua.nodeids.organizes
local browsename = ua.types.QualifiedName(1, "Office")
local typeidentifier = entity_id
local attr = ua.types.ObjectAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Office")
office_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                 reference_nodeid, browsename, typeidentifier, attr)

-- computer
local requested_nodeid = ua.nodeids.null
local parent_nodeid = office_id
local reference_nodeid = ua.nodeids.organizes
local browsename = ua.types.QualifiedName(1, "Computer")
local typeidentifier = entity_id
local attr = ua.types.ObjectAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Computer")
computer_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                   reference_nodeid, browsename, typeidentifier, attr)

local requested_nodeid = ua.nodeids.null
local parent_nodeid = computer_id
local reference_nodeid = ua.nodeids.hasproperty
local browsename = ua.types.QualifiedName(1, "Speed")
local typeidentifier = ua.nodeids.null
local attr = ua.types.VariableAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Speed")
server:addVariableNode(requested_nodeid, parent_nodeid,
                       reference_nodeid, browsename, typeidentifier, attr)

-- computer usb
local requested_nodeid = ua.nodeids.null
local parent_nodeid = computer_id
local reference_nodeid = ua.nodeids.organizes
local browsename = ua.types.QualifiedName(1, "ComputerUSB")
local typeidentifier = usb_id
local attr = ua.types.ObjectAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "ComputerUSB")
local computerusb_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                            reference_nodeid, browsename, typeidentifier, attr)

-- mouse
local requested_nodeid = ua.nodeids.null
local parent_nodeid = office_id
local reference_nodeid = ua.nodeids.organizes
local browsename = ua.types.QualifiedName(1, "Mouse")
local typeidentifier = entity_id
local attr = ua.types.ObjectAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Mouse")
mouse_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                reference_nodeid, browsename, typeidentifier, attr)

local requested_nodeid = ua.nodeids.null
local parent_nodeid = mouse_id
local reference_nodeid = ua.nodeids.hasproperty
local browsename = ua.types.QualifiedName(1, "Resolution")
local typeidentifier = ua.nodeids.null
local attr = ua.types.VariableAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Resolution")
server:addVariableNode(requested_nodeid, parent_nodeid,
                       reference_nodeid, browsename, typeidentifier, attr)

-- mouse usb
local requested_nodeid = ua.nodeids.null
local parent_nodeid = mouse_id
local reference_nodeid = ua.nodeids.organizes
local browsename = ua.types.QualifiedName(1, "MouseUSB")
local typeidentifier = usb_id
local attr = ua.types.ObjectAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "MouseUSB")
local mouseusb_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                         reference_nodeid, browsename, typeidentifier, attr)

-- mouse <--> computer usb
local source = mouseusb_id
local reftype = connectedwith_id
local target = computerusb_id
local isforward = true
server:addReference(source, reftype, target, isforward)

-- shells
local requested_nodeid = ua.nodeids.null
local parent_nodeid = ua.nodeids.objects
local reference_nodeid = ua.nodeids.organizes
local browsename = ua.types.QualifiedName(1, "Administration Shells")
local typeidentifier = ua.nodeids.foldertype
local attr = ua.types.ObjectAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Administration Shells")
shells_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                 reference_nodeid, browsename, typeidentifier, attr)

-- computer admin shell
local requested_nodeid = ua.nodeids.null
local parent_nodeid = shells_id
local reference_nodeid = ua.nodeids.organizes
local browsename = ua.types.QualifiedName(1, "Computer Admin Shell")
local typeidentifier = as_id
local attr = ua.types.ObjectAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Computer Admin Shell")
cas_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                 reference_nodeid, browsename, typeidentifier, attr)

-- computer admin shell -> computer
local source = computer_id
local reftype = administrates_id
local target = cas_id
local isforward = false
server:addReference(source, reftype, target, isforward)

-- add method
function test(server, objectid, arguments)
   print("here")
   return 1,2
end
local requested_nodeid = ua.nodeids.null
local parent_nodeid = ua.nodeids.objects
local reference_nodeid = ua.nodeids.hascomponent
local browsename = ua.types.QualifiedName(1, "Test Method")
local attr = ua.types.MethodAttributes()
attr.displayname = ua.types.LocalizedText("en_US", "Test Method")
attr.executable = true
cas_id = server:addMethodNode(requested_nodeid, parent_nodeid,
                              reference_nodeid, browsename, attr, test, {}, {})

-- start the server
server:start()

-- don't stop the server until somebody types 'q'
while true do
   input = io.read("*l")
   if input == "q" then return end
end

server:stop()
