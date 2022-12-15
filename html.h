char dashboard[] PROGMEM = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Final Exam - Isaac Laryea</title>
    <style> @import url('https://fonts.googleapis.com/css2?family=Titillium+Web:ital,wght@0,200;0,300;0,400;0,600;0,700;1,200;1,300&display=swap'); </style>
    <style>
        .center-header{
            display: flex;
            justify-content: center;
            align-items: center;
            color: #05445E;
        }
        .row{
            margin: 1rem 3rem;
            display: flex;
            justify-content: space-evenly;
        }
        .row-vals{
            margin: 1rem 3rem;
            display: flex;
            justify-content: space-evenly;
        }
        .col{
            margin: 1rem 1rem;
            display: flex;
            flex-direction: column;
            justify-content: center;
        }
        .u{
            text-decoration: underline;
        }
        p,span{
            font-weight: 300;
            text-transform: uppercase;
            text-align: center;
            color: #05445E;
        }
        .no-top{
            margin-top: 0px;
            margin-bottom: 0px;
        }
        .center{
            justify-content: center;
        }
        .fan-btn{
            background-color: #05445E;
            color: white;
            padding: 1rem;
            cursor: pointer;
            border-radius: 5px;
        }
        html{
            font-family: 'Titillium Web', sans-serif;
        }
    </style>
</head>
<body>
    <div class="center-header"><h2>IoT final project - Isaac Laryea</h2></div>
    <div class="col">
        <p class="u">AC fan control panel</p>
        <div class="row">
            <div class="fan-btn">Start AC fan</div>
            <div class="fan-btn">Stop AC fan</div>
        </div>
    </div>
    <div class="col">
        <p class="u">Temperature values</p>
        <div class="row center">
            <span>Current value:</span> &nbsp; <span id="current"></span> <span>°C</span>
        </div>
        <div class="row center"><div class="fan-btn">Display latest values</div></div>
        <div class="row-vals no-top">
            <ol>
                <li id="one">1</li>
                <li id="two">2</li>
                <li id="three">3</li>
                <li id="four">4</li>
                <li id="five">5</li>
            </ol>
            <ol start="6">
                <li id="six">6</li>
                <li id="seven">7</li>
                <li id="eigth">8</li>
                <li id="nine">9</li>
                <li id="ten">10</li>
            </ol>
        </div>
    </div>
</body>
</html>
)";
