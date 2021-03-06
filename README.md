# GTA V Racing AI / Direct Control Test

[![Build status](https://ci.appveyor.com/api/projects/status/1w2iarcia2qyfi9e/branch/master?svg=true)](https://ci.appveyor.com/project/E66666666/gtavdirectcontrol/branch/master)

This project has multiple goals, but is mostly to experiment and have fun
with.

1. This is a testbed for interaction with vehicles in GTA V's
world.
2. Player input and smoothing simulation, much like what Rockstar implented
for the player inputs.
3. Application of this on a custom AI to simulate realistic opponents.
4. Improving custom AI behavior for competetive racing.

This project does not have any relationship with Eddlm's
[`Advanced Racing System`](https://www.gta5-mods.com/scripts/autosport-racing-system)
, aside from me taking inspiration of the AI opponents
challenge, and this project does not have the same aspirations either. Just an
experiment for AI performance.

## Vehicle interaction

This part was easy, as it just requires setting the relevant fields in the
vehicle object. Any ped driver or player driver does override input, so it's
important to control an empty vehicle or a  vehicle with an untasked ped.

## Player input simulation

When a player applies a steering input to the game, the vehicle seems to move
as good as it can without understeering (front wheels steering more than their)
grip allows, and the vehicle is robust against minor pertubations. This
results in what is experienced as pleasant, predictable and fun gameplay.

Simply setting the angles of the vehicle wheels does not yield the same result,
as the vehicle loses all of its stability and will oversteer on input. This
can be noticed as a player when the code is patched out, which
[InfamousSabre's Custom Steering](https://www.gta5-mods.com/scripts/custom-steering-infamoussabre)
does. Mapping the inputs depending on speed only partially solves the
understeer issue, so another approach is needed.

The solution can be found by drawing some vehicle physics properties. With
natives some physics properties can be read, of which vehicle velocity vector
and rotation vector are most important. Drawing these, added to the position
of the vehicle yields in two points, which can be described as "predicted
position in 1 second" (units are in meters and m/s). Combining the relative
rotation and velocity vectors yields in one predicted position that shifts
according to player input.

This visualization tool shows that on player input, these points shift and
hold relative position, as the vehicle starts moving to the position. From
this, it can be speculated that the player input is used to control that
point, instead of the wheels directly. The wheels just move according to what
is apparently a control loop that wants to change the vehicle movement
direction.

With no input, the vehicle will correct to drive straight. With any steering
input, the vehicle will correct to follow the intended direction.

An implementation of this is present in this project, and shows comparable
performance to native game input, with comparable stability and
manoeuvrability.

## AI

"Actually just a bunch of `if` statements."

AI went through a bunch of iterations for different features. This is the
chronological order, which can also be derived from the commit history:

* Basic player stalking to test for stability and code-based inputs.
* Basic path following capability.
* Multiple AI instances.
* Overtaking AI.

The AI has three different look-ahead points, all of which are based on
the current speed. The faster the AI goes, the further away the
look-ahead point is. From far to closest: Throttle, brake and steering
look-ahead.

AI speed, distance to those points, angle between AI and those points, angle
between throttle and brake look-ahead all take part in deciding whether to
accelerate, coast or brake, with everything happening in a continuous way.

Steering input uses the input smoothing so the setpoint follows the track.
Various tidbits of code take car of track limits, corner apex prediction,
angle of approach and more track-based logic, which is to keep the AI instance
within track bounds. This combines into fast, consistent racers which perform
under all world conditions.

Steering input is taken over by overtaking logic, which is still in the works.

## Tracks

Tracks are loaded from either json or xml. Eddlm's tracks can be used. This
script also has basic track recording, which outputs json.

# Playing

Use the cheat console (`tilde/backtick`) to enter the commands.

Basic input:

* `cc` - Control current vehicle. Drive with IJKL as WASD, U as reverse, I as handbrake. A second controller can also be used. Toggles on and off.
* `pp` - Move to passenger position. Repeat to get back in drivers' seat.

AI control:

* `makeai1` - Enable AI on current vehicle.
* `makeai0` - Disable AI on current vehicle.
* `addai` - Add AI vehicles. First prompt is amount, second is vehicle model.
* `startai` - Start AI control.
* `stopai` - Stop AI control.
* `fixai` - Fix AI vehicles.
* `delai` - Remove AI from all vehicles.
* `dbgai1` - Enable AI debug info.
* `dbgai0` - Disable AI debug info.
* `dbgtxt1` - Enable additional AI debug info text box.
* `dbgtxt-` - Disable additional AI debug info text box.
* `dbgthis` - Toggle debug info state on current vehicle.
* `loadconfig` - Override config for current occupied vehicle.

Race track control:

* `startrecord` - Start recording current path as track. 1m node distance.
* `stoprecord` - Stop recording.
* `cleartrack` - Delete all (loaded) nodes.
* `loadtrack` - Show list of tracks and show prompt. Supports Eddlm's XML format and my JSON alternative.
* `savetrack` - Save track to JSON file.
* `reversetrack` - Reverse loaded track nodes.

Misc/debug commands:

* `dbgtrack` - Toggle track drawing.
* `reloadsettings` - Reload general settings and AI configs.

# Building

You'll need [Script Hook V SDK](http://www.dev-c.com/gtav/scripthookv/). Clone
this in the folder where you extracted the contents to ScriptHookV_SDK.

# License

Copyright (c) 2019 ikt

* You may use, copy and modify the software for personal or educational
  purposes.
* You may re-use parts of the software for your own projects, as long as said
  projects are not sold commercially.
* You may not publish or distribute the software as-is, or in a derived form
  with significant overlap.

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
