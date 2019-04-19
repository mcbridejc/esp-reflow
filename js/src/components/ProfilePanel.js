import React, {Component} from "react";
import PropTypes from 'prop-types';
import { withStyles } from '@material-ui/core/styles';
import Button from '@material-ui/core/Button';
import CircularProgress from '@material-ui/core/CircularProgress';
import Dialog from '@material-ui/core/Dialog';
import DialogActions from '@material-ui/core/DialogActions';
import DialogContent from '@material-ui/core/DialogContent';
import DialogTitle from '@material-ui/core/DialogTitle';
import Fab from '@material-ui/core/Fab';
import AddIcon from '@material-ui/icons/Add';
import DeleteIcon from '@material-ui/icons/Delete';
import Grid from '@material-ui/core/Grid';
import Paper from '@material-ui/core/Paper';
import Table from '@material-ui/core/Table';
import TableBody from '@material-ui/core/TableBody';
import TableCell from '@material-ui/core/TableCell';
import TableHead from '@material-ui/core/TableHead';
import TableRow from '@material-ui/core/TableRow';
import TextField from '@material-ui/core/TextField';
import Checkbox from '@material-ui/core/Checkbox';

import ProfileGraph from './ProfileGraph.js';

// Lame way to make a deep copy of an object
function jsonCopy(src) {
  return JSON.parse(JSON.stringify(src));
}

const styles = theme => ({
  root: {},
  paper: {
    textAlign: "center",
    verticalAlign: "middle",
  },
  gaugeLabel: {
    fontSize: "1.1em",
    paddingTop: "1em"
  },
  gaugeValue: {
    fontSize: "1.5em",
    fontWeight: "bold",
    padding: "1em",
  },
  chart: {
    height: "300px",
  },
  cancelSave: {
    textAlign: "center"
  }
});


class ProfileEditor extends Component {
  constructor(props) {
    super(props);
    this.state = {profile: props.profile};

    this.handleNameChange = this.handleNameChange.bind(this);
    this.addStep = this.addStep.bind(this);
  }

  handleNameChange(event) {
    let oldState = this.state;
    oldState.profile.name = event.target.value;
    this.setState(oldState);
  }
  handleTempChange(i) {
    return (event) => {
      let oldState = this.state;
      oldState.profile.steps[i].temp = parseInt(event.target.value);
      this.setState(oldState);
    }
  }

  handleDurationChange(i) {
    return (event) => {
      let oldState = this.state;
      oldState.profile.steps[i].duration = parseInt(event.target.value);
      this.setState(oldState);
    }
  }

  handleRampChange(i) {
    return (event) => {
      let oldState = this.state;
      oldState.profile.steps[i].ramp = event.target.checked ? 1 : 0;
      this.setState(oldState);
    }
  }

  handleDelete(i) {
    return (event) => {
      let oldState = this.state;
      oldState.profile.steps.splice(i, 1); // remove element i
      this.setState(oldState);
    }
  }

  addStep() {
    let oldState = this.state;
    oldState.profile.steps.push({temp: 0, duration: 0, ramp: 0});
    this.setState(oldState);
  }

  render() {
    let {classes} = this.props;
    return (
      <Dialog maxWidth='md' fullWidth={true} open={true}>
        <DialogTitle>Edit profile</DialogTitle>
        <DialogContent>
          <Grid container justify="center" spacing={16}>
            <Grid item xs={12}>
              <TextField
                    id="profile-name"
                    label="Name"
                    value={this.state.profile.name}
                    onChange={this.handleNameChange}
                    margin="normal"
                    variant="outlined"
                  />
              <Table>
                <TableHead>
                  <TableCell>Step #</TableCell>
                  <TableCell>Temperature (C)</TableCell>
                  <TableCell>Duration</TableCell>
                  <TableCell>Ramp</TableCell>
                  <TableCell>Actions</TableCell>
                </TableHead>
                <TableBody>
                  {this.state.profile.steps.map( (step, i) => (
                    <TableRow key={i}>
                      <TableCell>{i}</TableCell>
                      <TableCell>
                        <TextField
                          id={"temp-" + i}
                          value={step.temp}
                          onChange={this.handleTempChange(i)}
                          margin="normal"
                          type="number" />
                      </TableCell>
                      <TableCell>
                        <TextField
                          id={"duration-" + i}
                          value={step.duration}
                          onChange={this.handleDurationChange(i)}
                          margin="normal"
                          type="number" />
                      </TableCell>
                      <TableCell>
                        <Checkbox id={"ramp-" + i} checked={step.ramp == 1} onChange={this.handleRampChange(i)} />
                      </TableCell>
                      <TableCell>
                        <Fab aria-label="Delete" onClick={this.handleDelete(i)}>
                          <DeleteIcon />
                        </Fab>
                      </TableCell>
                    </TableRow>
                  ))}
                  <TableRow>
                    <TableCell colSpan={5} align="right">
                      <Fab color="primary" aria-label="Add" onClick={this.addStep}>
                        <AddIcon />
                      </Fab>
                    </TableCell>
                  </TableRow>
                </TableBody>
              </Table>
            </Grid>
            <Grid item xs={12}>
              <ProfileGraph profile={this.props.profile} />
            </Grid>
          </Grid>
        </DialogContent>
        <DialogActions>
          <Button variant="contained" color="primary" onClick={this.props.onCancel}>Cancel</Button>
          <Button variant="contained" color="primary" onClick={this.props.onSave}>Save</Button>
        </DialogActions>
      </Dialog>     
    );
  }
}

let ProfileEditorStyled = withStyles(styles)(ProfileEditor)

class ProfileList extends Component {

  constructor(props) {
    super(props);

    this.state = {
      editing: false,
      editProfileName: null,
      editProfile: null,
    }

    this.handleEdit = this.handleEdit.bind(this);
  }
  
  handleEdit(name) {
    var profileToEdit;
    if(name) {
      profileToEdit = jsonCopy(this.props.profiles.find((p) => { return p.name == name; }));
    } else {
      profileToEdit = {name: "NewProfile", steps: []};
    }
    this.setState({
      editing: true, 
      editProfile: profileToEdit,
      editProfileName: name
    });
  }

  handleCancelEdit() {
    this.setState({editing: false});
  }

  handleSave() {
    this.setState({editing: false});
    this.props.onChange(this.state.editProfileName, this.state.editProfile);
  }

  render() {
    let {classes} = this.props;
    if(this.state.editing) {
      return (
        <ProfileEditorStyled
          open={true}
          onSave={this.handleSave.bind(this)}
          onCancel={this.handleCancelEdit.bind(this)}
          profile={this.state.editProfile} />
      );
    } else if(this.props.profiles) {
      return (
        <Grid container spacing={16}>
          <Grid item xs={12}>
            <Table>
              <TableHead>
                <TableRow>
                  <TableCell>Profile Name</TableCell>
                  <TableCell></TableCell>
                  <TableCell></TableCell>
                </TableRow>
              </TableHead>
              <TableBody>
                {this.props.profiles.map(p => (
                  <TableRow key={p.name}>
                    <TableCell>{p.name}</TableCell>
                    <TableCell><Button onClick={() => this.props.onActivate(p.name)}>Activate</Button></TableCell>
                    <TableCell><Button onClick={() => this.handleEdit(p.name)}>Edit</Button></TableCell>
                  </TableRow>
                ))}
              </TableBody>
            </Table>
          </Grid>
          <Grid item xs={12}>
            <Button color="primary" variant="contained" onClick={() => this.handleEdit(null)}>Create New Profile</Button>
          </Grid>

        </Grid>
      )
    } else {
      return <CircularProgress className={classes.spinner}></CircularProgress>
    } 
  }
}
let ProfilesListStyled = withStyles(styles)(ProfileList);

class ProfilePanel extends Component {
  constructor(props) {
    super(props);

    this.handleClickEdit = this.handleClickEdit.bind(this);
    this.handleClose = this.handleClose.bind(this);
    this.activateProfile = this.activateProfile.bind(this);
    this.saveProfile = this.saveProfile.bind(this);
    this.state = { openDialog: false, profiles: null};
    this.getActiveProfile();
  }
  
  getActiveProfile() {
    // Clear any pending timeouts (in case we've called this an extra time to accelerate)
    clearTimeout(this.slowTimer);

    fetch('/api/profiles/active')
    .then(res => res.json())
    .then(
      (result) => {
        this.setState({activeProfile: result});
      },
      (error) => {
        console.log("Error loading active profile: ", error.message);
      }
    )
    .finally(() => {
      this.slowTimer = setTimeout(() => this.getActiveProfile(), 5000);
    })
  }

  getProfiles() {
    fetch('/api/profiles')
    .then(res => res.json())
    .then(
      (result) => {
        this.setState({profiles: result});
      },
      (error) => {
        console.log("Error loading profiles: ", error.message);
      }
    )
  }

  activateProfile(profileName) {
    this.setState({openDialog: false});
    fetch('/api/activate/'+profileName)
    .then(
      (result) => {
        this.getActiveProfile();
      }
    )
  }

  saveProfile(existingName, newProfile) {
    if(existingName) {
      fetch('/api/profiles/' + existingName, {
        method: 'put',
        body: JSON.stringify(newProfile),
        headers: {'Content-Type': 'application/json'}
      })
      .then(res => res.json())
      .then(
        (result) => {
          this.setState({profiles: result});
        }, 
        (error) => {
          alert("Error saving profile: ", error.message);
          console.log("Error saving profile: ", error.message);
        }
      )
    } else {
      fetch('/api/profiles', {
        method: 'post',
        body: JSON.stringify(newProfile),
        headers: {'Content-Type': 'application/json'}
      })
      .then(res => res.json())
      .then(
        (result) => {
          this.setState({profiles: result});
        }, 
        (error) => {
          alert("Error creating profile: ", error.message);
          console.log("Error creating profile: ", error.message);
        }
      )
    }
  }

  handleClickEdit() {
    this.setState({profiles: null, openDialog: true});
    this.getProfiles();
  }

  handleClose() {
    this.setState({openDialog: false});
  }

  render() {
    let {classes} = this.props;
    return (
      <Paper className={classes.paper}>
        <h2>Active Profile: {this.state.activeProfile && this.state.activeProfile.name || "None"}</h2>
        <Button onClick={this.handleClickEdit}>Change</Button>
        <ProfileGraph profile={this.state.activeProfile}></ProfileGraph>
        <Dialog open={this.state.openDialog} onClose={this.handleClose}>
          <DialogTitle>Manage profiles</DialogTitle>
          <DialogContent>
            <ProfilesListStyled profiles={this.state.profiles} onActivate={this.activateProfile} onChange={this.saveProfile}></ProfilesListStyled>

          </DialogContent>
          <DialogActions>
            <Button color="primary" variant="contained" onClick={this.handleClose}>Done</Button>
          </DialogActions>
        </Dialog>
      </Paper>
    );
  }
}

ProfilePanel.propTypes = {
  classes: PropTypes.object.isRequired,
  profile: PropTypes.object
};

export default withStyles(styles)(ProfilePanel);