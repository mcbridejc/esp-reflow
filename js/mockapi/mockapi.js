// Mock routes for reflow HTTP API
// For developing the javascript front-end sans device

/* 

Index of all profiles
GET /profiles

Create a new profile
POST /profiles/

Modify existing profile
PUT /profiles/<name>

Delete a profile
DELETE /profiles/<name>

A profile is just a list of steps of the form: 

{
  'name': <profile_name>, // Unique identifier
  'steps': [
    {
      'temp': <temp in degC>,
      'duration': <Duration of step in seconds>,
      'ramp': <1 if step is a ramping step, 0 if it is a acquire and hold step>
    }, ...
  ]  
}

*/ 

// Create an initial list of profiles that will be modified by api calls
profiles = [
  {
    'name': 'lead',
    'steps': [
      {
        'temp': 150,
        'duration': 60,
        'ramp': 1,
      },
      {
        'temp': 165,
        'duration': 90,
        'ramp': 1
      },
      {
        'temp': 225,
        'duration': 40,
        'ramp': 0
      },
      {
        'temp': 150,
        'duration': 25,
        'ramp': 0
      },
    ]
  },
  {
    'name': 'lead-free',
    'steps': [
      {
        'temp': 150,
        'duration': 60,
        'ramp': 1,
      },
      {
        'temp': 180,
        'duration': 120,
        'ramp': 1
      },
      {
        'temp': 245,
        'duration': 40,
        'ramp': 0
      },
      {
        'temp': 150,
        'duration': 25,
        'ramp': 0
      },
    ]
  }
]

activeProfile = profiles[0];

profilesIndex = (request, response) => {
  response.json(profiles);
}

profilesCreate = (request, response) => {
  let reqjson = request.body;
  let found = profiles.some((p) => {return p.name == reqjson['name']; })
  if(found) { 
    response.status(400).json({'message': 'Profile already exists'});
  } else {
    profiles.push(reqjson);
    response.json(profiles);
  }
}

profilesUpdate = (request, response) => {
  let reqjson = request.body;
  let reqname = request.params.name;
  let updateProfile = profiles.find((p) => { return p.name == reqname; });
  let updateIndex = profiles.indexOf(updateProfile);
  if(updateProfile) {
    profiles[updateIndex] = reqjson;
    response.json(profiles);
  } else {
    response.status(404).json({'message': 'Profile not found'});
  }
}

profilesDestroy = (request, response) => {
  reqname = request.params.name;
  deleteProfile = profiles.find((p) => { return p.name == reqname; });
  deleteIndex = profiles.indexOf(deleteProfile);
  if(deleteIndex >= 0) {
    profiles.splice(deleteIndex, 1);
    response.json(profiles);
  } else {
    response.status(404).json({"message": "Not found"});
  }
}

profilesActive = (request, response) => {
  response.json(activeProfile);
}

activate = (request, response) => {
  reqname = request.params.name;
  profile = profiles.find((p) => { return p.name == reqname; });
  if(profile) {
    activeProfile = profile;
    response.json({"message": "Activated profile"});
  } else {
    response.status(404).json({"message": "Profile not found"});
  }
}


status = (request, response) => {
  response.json({
    "temperature": 101,
    "targetTemperature": 105,
    "profileStage": 0,
    "profileElapsedTime": 1,
    "output": 50,
    "state": "idle",
  })
}

module.exports = {
  'profiles': {
    'index': profilesIndex,
    'create': profilesCreate,
    'update': profilesUpdate,
    'destroy': profilesDestroy,
    'active': profilesActive
  },
  'status': status,
  'activate': activate
};

